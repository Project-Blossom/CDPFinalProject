#include "Item/BoltAnchorActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ABoltAnchorActor::ABoltAnchorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    BoltMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoltMesh"));
    BoltMesh->SetupAttachment(Root);
    BoltMesh->SetSimulatePhysics(false);
    BoltMesh->SetMobility(EComponentMobility::Static);
    BoltMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    HandSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HandSnapPoint"));
    HandSnapPoint->SetupAttachment(Root);

    RopeAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RopeAttachPoint"));
    RopeAttachPoint->SetupAttachment(Root);

    CurrentDurability = MaxDurability;
}

void ABoltAnchorActor::BeginPlay()
{
    Super::BeginPlay();
    CurrentDurability = MaxDurability;
}

FVector ABoltAnchorActor::GetRopeAttachWorldLocation() const
{
    return RopeAttachPoint ? RopeAttachPoint->GetComponentLocation() : GetActorLocation();
}

FVector ABoltAnchorActor::GetHandSnapWorldLocation() const
{
    return HandSnapPoint ? HandSnapPoint->GetComponentLocation() : GetActorLocation();
}

bool ABoltAnchorActor::CanAttachSafetyLine() const
{
    return !bSafetyLineAttached && !IsBroken();
}

void ABoltAnchorActor::SetSafetyLineAttached(bool bAttached, AActor* InUser)
{
    bSafetyLineAttached = bAttached;
    AttachedUser = bAttached ? InUser : nullptr;
}

bool ABoltAnchorActor::ConsumeDurability(float Amount)
{
    if (Amount <= 0.0f)
    {
        return !IsBroken();
    }

    CurrentDurability = FMath::Max(0.0f, CurrentDurability - Amount);
    return !IsBroken();
}