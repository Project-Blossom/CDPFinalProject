#include "Item/ItemSpawner.h"

#include "Item/ItemDropActor.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AItemSpawner::AItemSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    SetRootComponent(SpawnVolume);

    SpawnVolume->SetBoxExtent(FVector(500.0f, 500.0f, 100.0f));
    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpawnVolume->SetHiddenInGame(true);

    DefaultDropActorClass = AItemDropActor::StaticClass();
}

void AItemSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSpawnOnBeginPlay)
    {
        SpawnItems();
    }
}

void AItemSpawner::SpawnItems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (bClearExistingDropsBeforeSpawn)
    {
        ClearSpawnedItems();
    }

    FRandomStream Stream;
    if (bUseFixedRandomSeed)
    {
        Stream.Initialize(RandomSeed);
    }
    else
    {
        Stream.GenerateNewSeed();
    }

    if (bDrawDebugSpawnArea && SpawnVolume)
    {
        DrawDebugBox(
            World,
            SpawnVolume->GetComponentLocation(),
            SpawnVolume->GetScaledBoxExtent(),
            SpawnVolume->GetComponentQuat(),
            FColor::Green,
            false,
            5.0f,
            0,
            3.0f
        );
    }

    for (const FItemSpawnEntry& Entry : ItemsToSpawn)
    {
        if (Entry.ItemId == NAME_None || Entry.SpawnCount <= 0)
        {
            continue;
        }

        TSubclassOf<AItemDropActor> ClassToSpawn = Entry.DropActorClass ? Entry.DropActorClass : DefaultDropActorClass;
        if (!ClassToSpawn)
        {
            continue;
        }

        for (int32 i = 0; i < Entry.SpawnCount; ++i)
        {
            FTransform SpawnTransform;
            if (!TryFindSpawnTransform(Stream, SpawnTransform))
            {
                UE_LOG(LogTemp, Warning, TEXT("ItemSpawner '%s': failed to find spawn transform for %s"),
                    *GetName(), *Entry.ItemId.ToString());
                continue;
            }

            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AItemDropActor* Drop = World->SpawnActor<AItemDropActor>(ClassToSpawn, SpawnTransform, Params);
            if (!Drop)
            {
                continue;
            }

            const int32 DropCount = RollDropCount(Entry, Stream);
            Drop->InitializeDropWithMesh(Entry.ItemId, DropCount, Entry.VisualMesh);
            SpawnedDrops.Add(Drop);
        }
    }
}

void AItemSpawner::ClearSpawnedItems()
{
    for (AItemDropActor* Drop : SpawnedDrops)
    {
        if (IsValid(Drop))
        {
            Drop->Destroy();
        }
    }

    SpawnedDrops.Reset();
}

bool AItemSpawner::TryFindSpawnTransform(FRandomStream& Stream, FTransform& OutTransform) const
{
    for (int32 Attempt = 0; Attempt < MaxAttemptsPerDrop; ++Attempt)
    {
        FVector Candidate = GetRandomPointInSpawnVolume(Stream);

        if (bSnapToGround && GetWorld())
        {
            const FVector TraceStart = Candidate + FVector::UpVector * GroundTraceUpCm;
            const FVector TraceEnd = Candidate - FVector::UpVector * GroundTraceDownCm;

            FHitResult Hit;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(ItemSpawnerGroundTrace), false, this);

            const bool bHit = GetWorld()->LineTraceSingleByChannel(
                Hit,
                TraceStart,
                TraceEnd,
                GroundTraceChannel,
                Params
            );

            if (!bHit || !Hit.bBlockingHit)
            {
                continue;
            }

            Candidate = Hit.ImpactPoint + Hit.ImpactNormal.GetSafeNormal() * GroundSpawnOffsetCm;
        }

        if (!IsFarEnoughFromExistingDrops(Candidate))
        {
            continue;
        }

        OutTransform = FTransform(FRotator::ZeroRotator, Candidate, FVector::OneVector);
        return true;
    }

    return false;
}

FVector AItemSpawner::GetRandomPointInSpawnVolume(FRandomStream& Stream) const
{
    if (!SpawnVolume)
    {
        return GetActorLocation();
    }

    const FVector Extent = SpawnVolume->GetScaledBoxExtent();
    const FVector Local(
        Stream.FRandRange(-Extent.X, Extent.X),
        Stream.FRandRange(-Extent.Y, Extent.Y),
        Stream.FRandRange(-Extent.Z, Extent.Z)
    );

    return SpawnVolume->GetComponentTransform().TransformPosition(Local);
}

bool AItemSpawner::IsFarEnoughFromExistingDrops(const FVector& Location) const
{
    if (MinDistanceBetweenDropsCm <= 0.0f)
    {
        return true;
    }

    for (AItemDropActor* Drop : SpawnedDrops)
    {
        if (!IsValid(Drop))
        {
            continue;
        }

        if (FVector::Dist(Drop->GetActorLocation(), Location) < MinDistanceBetweenDropsCm)
        {
            return false;
        }
    }

    return true;
}

int32 AItemSpawner::RollDropCount(const FItemSpawnEntry& Entry, FRandomStream& Stream) const
{
    const int32 MinCount = FMath::Max(1, Entry.CountMin);
    const int32 MaxCount = FMath::Max(MinCount, Entry.CountMax);
    return Stream.RandRange(MinCount, MaxCount);
}