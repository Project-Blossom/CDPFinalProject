#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleMeshPlacerActor.generated.h"

class UStaticMeshComponent;

/**
 * 레벨에 배치하면 StaticMeshComponent 하나만 있는 단순 액터
 * - 모양 바꾸고 싶으면 Details에서 MeshComponent의 Static Mesh만 바꾸면 됨
 */
UCLASS()
class SIMPLEMESHPLACER_API ASimpleMeshPlacerActor : public AActor
{
    GENERATED_BODY()

public:
    ASimpleMeshPlacerActor();

protected:
    // 이 컴포넌트에 메쉬를 직접 할당해서 사용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMeshComponent* MeshComponent;
};