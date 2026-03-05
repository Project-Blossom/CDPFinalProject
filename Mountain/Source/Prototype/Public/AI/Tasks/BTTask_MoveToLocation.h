#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToLocation.generated.h"

/**
 * Task: Blackboard의 Vector 위치로 이동
 */
UCLASS()
class PROTOTYPE_API UBTTask_MoveToLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToLocation();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetLocationKey;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptanceRadius = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MoveSpeed = 300.0f;
};
