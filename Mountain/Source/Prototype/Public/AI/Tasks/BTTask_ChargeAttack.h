#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChargeAttack.generated.h"

/**
 * Task: 돌진 공격 준비 (조준)
 * ChargeTime 동안 플레이어를 향해 회전
 */
UCLASS()
class PROTOTYPE_API UBTTask_ChargeAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ChargeAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;
    
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector AttackTargetLocationKey;
    
    UPROPERTY(EditAnywhere, Category = "Attack")
    float ChargeTime = 1.0f;

private:
    float ChargeStartTime = 0.0f;
};
