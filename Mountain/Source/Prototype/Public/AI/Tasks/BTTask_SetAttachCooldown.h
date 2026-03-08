#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetAttachCooldown.generated.h"

UCLASS()
class PROTOTYPE_API UBTTask_SetAttachCooldown : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_SetAttachCooldown();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastAttachTimeKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bCanAttachKey;
};
