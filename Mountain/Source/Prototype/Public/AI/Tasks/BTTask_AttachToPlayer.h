#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttachToPlayer.generated.h"

UCLASS()
class PROTOTYPE_API UBTTask_AttachToPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_AttachToPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;
};
