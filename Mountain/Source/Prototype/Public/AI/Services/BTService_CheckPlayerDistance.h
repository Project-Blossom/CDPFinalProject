#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckPlayerDistance.generated.h"

/**
 * Service: 플레이어와의 거리를 체크하여 Blackboard 값 설정
 * bPlayerNearby = 가까이 있으면 true, 멀면 false
 */
UCLASS()
class PROTOTYPE_API UBTService_CheckPlayerDistance : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_CheckPlayerDistance();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // Blackboard Keys
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bPlayerNearbyKey;

    // 가까운 거리 기준 (cm)
    UPROPERTY(EditAnywhere, Category = "Distance", meta = (ClampMin = "100", ClampMax = "2000"))
    float NearbyDistance = 800.0f;
};
