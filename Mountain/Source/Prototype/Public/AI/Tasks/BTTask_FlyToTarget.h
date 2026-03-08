#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FlyToTarget.generated.h"

/**
 * Task: FlyingMonster가 목표 위치로 비행
 * FlyingMonster의 FlyToLocation 함수 사용
 */
UCLASS()
class PROTOTYPE_API UBTTask_FlyToTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FlyToTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptanceRadius = 100.0f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float FlightSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bAvoidObstacles = true;

    // Attack (FlyingAttacker용)
    UPROPERTY(EditAnywhere, Category = "Attack")
    float InsanityDamage = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float HitCheckRadius = 150.0f;

    // 이번 Task 실행 중 이미 플레이어를 타격했는지 여부
    bool bHasHitPlayerThisTask = false;
};
