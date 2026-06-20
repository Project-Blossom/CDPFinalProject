#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToPlayer.generated.h"

UCLASS()
class PROTOTYPE_API UBTTask_MoveToPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptanceRadius = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MovementSpeed = 400.0f;

    // 플레이어까지의 거리가 이 값을 초과하면 추적을 포기하고 Failed를 반환한다.
    // 0 이하로 두면 기존과 동일하게 거리 제한 없이 무한 추적한다.
    // FlyingPlatform이 WallCrawler를 운반하며 플레이어를 어디서든 쫓아가는 문제를 막기 위해 추가.
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxTrackingDistance = 0.0f;
};
