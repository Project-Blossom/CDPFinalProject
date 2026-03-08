#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PursuePlayer.generated.h"

UCLASS()
class PROTOTYPE_API UBTTask_PursuePlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PursuePlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    UPROPERTY(EditAnywhere, Category = "Pursuit")
    float AttachRange = 150.0f;
};
