#include "Item/ItemDropActor.h"

#include "Components/SphereComponent.h"
#include "Item/InventoryComponent.h"

AItemDropActor::AItemDropActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    SetRootComponent(Sphere);

    Sphere->InitSphereRadius(50.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AItemDropActor::BeginPlay()
{
    Super::BeginPlay();

    if (Sphere)
    {
        Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItemDropActor::OnSphereBeginOverlap);
    }
}

void AItemDropActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    if (ItemId == NAME_None || Count <= 0) return;

    UInventoryComponent* Inv = OtherActor->FindComponentByClass<UInventoryComponent>();
    if (!Inv) return;

    if (Inv->TryAdd(ItemId, Count))
    {
        Destroy();
    }
}