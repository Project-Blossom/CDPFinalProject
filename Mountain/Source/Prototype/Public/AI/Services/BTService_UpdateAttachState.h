#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateAttachState.generated.h"

UCLASS()
class PROTOTYPE_API UBTService_UpdateAttachState : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateAttachState();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastAttachTimeKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bCanAttachKey;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttachCooldown = 5.0f;
};
