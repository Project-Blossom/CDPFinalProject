#include "Item/ItemDropActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"

namespace
{
    struct FDropPickedSound
    {
        USoundBase* Sound = nullptr;
        float VolumeMultiplier = 1.0f;
        float PitchMultiplier = 1.0f;
    };

    static FDropPickedSound SelectDropSoundVariant(const TArray<FItemSoundVariant>& Variants, USoundBase* FallbackSound, float FallbackVolumeMultiplier, float FallbackPitchMultiplier)
    {
        TArray<FDropPickedSound> ValidSounds;
        ValidSounds.Reserve(Variants.Num() + 1);

        if (FallbackSound)
        {
            ValidSounds.Add({ FallbackSound, FMath::Max(0.0f, FallbackVolumeMultiplier), FMath::Max(0.01f, FallbackPitchMultiplier) });
        }

        for (const FItemSoundVariant& Variant : Variants)
        {
            if (Variant.Sound)
            {
                ValidSounds.Add({ Variant.Sound.Get(), FMath::Max(0.0f, Variant.VolumeMultiplier), FMath::Max(0.01f, Variant.PitchMultiplier) });
            }
        }

        if (ValidSounds.Num() > 0)
        {
            return ValidSounds[FMath::RandRange(0, ValidSounds.Num() - 1)];
        }

        return FDropPickedSound();
    }

    static float ApplyDropManualDistanceVolume(const AActor* ListenerActor, float VolumeMultiplier, EItemSoundPlaybackMode PlaybackMode, const FVector& EventLocation)
    {
        const float BaseVolume = FMath::Max(0.0f, VolumeMultiplier);

        if (!ListenerActor || PlaybackMode == EItemSoundPlaybackMode::Play2D)
        {
            return BaseVolume;
        }

        const float DistanceCm = FVector::Dist(ListenerActor->GetActorLocation(), EventLocation);
        const float DistanceDenominator = 1.0f + (DistanceCm / 1000.0f);
        return BaseVolume / FMath::Max(1.0f, DistanceDenominator);
    }
}


AItemDropActor::AItemDropActor()
{
    PrimaryActorTick.bCanEverTick = true;

    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    SetRootComponent(PickupSphere);

    PickupSphere->InitSphereRadius(PickupRadiusCm);
    PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(PickupSphere);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemMesh->SetGenerateOverlapEvents(false);
}

void AItemDropActor::BeginPlay()
{
    Super::BeginPlay();

    if (PickupSphere)
    {
        PickupSphere->SetSphereRadius(PickupRadiusCm, true);
        PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemDropActor::OnSphereBeginOverlap);
    }

    if (ItemMesh)
    {
        InitialMeshRelativeLocation = ItemMesh->GetRelativeLocation();
    }
}

void AItemDropActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ItemMesh || (!bRotateInWorld && !bBobInWorld))
    {
        return;
    }

    VisualTime += DeltaTime;

    if (bRotateInWorld && !FMath::IsNearlyZero(RotationYawSpeedDegPerSec))
    {
        FRotator Rot = ItemMesh->GetRelativeRotation();
        Rot.Yaw += RotationYawSpeedDegPerSec * DeltaTime;
        ItemMesh->SetRelativeRotation(Rot);
    }

    if (bBobInWorld && !FMath::IsNearlyZero(BobAmplitudeCm))
    {
        FVector Loc = InitialMeshRelativeLocation;
        Loc.Z += FMath::Sin(VisualTime * BobSpeed) * BobAmplitudeCm;
        ItemMesh->SetRelativeLocation(Loc);
    }
}

void AItemDropActor::InitializeDrop(FName NewItemId, int32 NewCount)
{
    ItemId = NewItemId;
    Count = FMath::Max(1, NewCount);
}

void AItemDropActor::InitializeDropWithMesh(FName NewItemId, int32 NewCount, UStaticMesh* NewVisualMesh)
{
    InitializeDrop(NewItemId, NewCount);
    SetVisualMesh(NewVisualMesh);
}

void AItemDropActor::SetVisualMesh(UStaticMesh* NewVisualMesh)
{
    if (ItemMesh && NewVisualMesh)
    {
        ItemMesh->SetStaticMesh(NewVisualMesh);
    }
}

bool AItemDropActor::TryPickup(AActor* Picker)
{
    if (!Picker || ItemId == NAME_None || Count <= 0)
    {
        return false;
    }

    UInventoryComponent* Inv = Picker->FindComponentByClass<UInventoryComponent>();
    if (!Inv)
    {
        return false;
    }

    if (!Inv->CanAdd(ItemId, Count))
    {
        return false;
    }

    if (Inv->TryAdd(ItemId, Count))
    {
        const FDropPickedSound PickedSound = SelectDropSoundVariant(PickupSoundVariants, PickupSound, PickupSoundVolumeMultiplier, PickupSoundPitchMultiplier);
        if (PickedSound.Sound)
        {
            const FVector EventLocation = GetActorLocation();
            const float FinalVolume = ApplyDropManualDistanceVolume(Picker, PickedSound.VolumeMultiplier, PickupSoundPlaybackMode, EventLocation);

            switch (PickupSoundPlaybackMode)
            {
            case EItemSoundPlaybackMode::Play2D:
                UGameplayStatics::PlaySound2D(this, PickedSound.Sound, FinalVolume, PickedSound.PitchMultiplier);
                break;

            case EItemSoundPlaybackMode::UserLocation:
                UGameplayStatics::PlaySoundAtLocation(this, PickedSound.Sound, Picker->GetActorLocation(), FinalVolume, PickedSound.PitchMultiplier, 0.0f, PickupSoundAttenuation);
                break;

            case EItemSoundPlaybackMode::EventLocation:
            default:
                UGameplayStatics::PlaySoundAtLocation(this, PickedSound.Sound, EventLocation, FinalVolume, PickedSound.PitchMultiplier, 0.0f, PickupSoundAttenuation);
                break;
            }
        }

        Destroy();
        return true;
    }

    return false;
}

void AItemDropActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoPickupOnOverlap)
    {
        return;
    }

    TryPickup(OtherActor);
}