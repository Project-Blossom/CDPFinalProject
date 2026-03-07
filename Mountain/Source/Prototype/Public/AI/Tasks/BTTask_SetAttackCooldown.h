#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetAttackCooldown.generated.h"

/**
 * Task: 공격 쿨다운 시작
 * LastAttackTime을 현재 시간으로 설정
 */
UCLASS()
class PROTOTYPE_API UBTTask_SetAttackCooldown : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_SetAttackCooldown();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastAttackTimeKey;
};
