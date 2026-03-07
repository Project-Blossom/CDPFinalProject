#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateAttackState.generated.h"

/**
 * Service: FlyingAttacker의 공격 상태 업데이트
 * 거리 체크, 쿨다운 체크, Blackboard 동기화
 */
UCLASS()
class PROTOTYPE_API UBTService_UpdateAttackState : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateAttackState();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;
    
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastAttackTimeKey;
    
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bCanAttackKey;
    
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackCooldown = 5.0f;
};
