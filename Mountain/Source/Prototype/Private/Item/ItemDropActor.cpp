#include "Item/ItemDropActor.h"

#include "Components/SphereComponent.h"
#include "Item/InventoryComponent.h"
#include "Item/ItemDefinition.h"

AItemDropActor::AItemDropActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    RootComponent = Sphere;

    Sphere->SetSphereRadius(60.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionObjectType(ECC_WorldDynamic);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItemDropActor::OnOverlap);
}

void AItemDropActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !ItemDef || Count <= 0) return;

    UInventoryComponent* Inv = OtherActor->FindComponentByClass<UInventoryComponent>();
    if (!Inv) return;

    const bool bAddedAll = Inv->TryAddByDefinition(ItemDef, Count);
    if (bAddedAll)
    {
        Destroy();
    }
}