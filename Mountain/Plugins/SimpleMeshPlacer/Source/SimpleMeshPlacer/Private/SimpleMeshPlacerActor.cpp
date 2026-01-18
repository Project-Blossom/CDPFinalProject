#include "SimpleMeshPlacerActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

ASimpleMeshPlacerActor::ASimpleMeshPlacerActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // 메쉬 컴포넌트를 루트로 사용
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}