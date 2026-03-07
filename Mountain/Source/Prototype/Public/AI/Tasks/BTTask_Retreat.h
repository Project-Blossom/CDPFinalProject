#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Retreat.generated.h"

/**
 * Task: 플레이어로부터 후퇴
 * SafeDistance까지 거리 유지
 */
UCLASS()
class PROTOTYPE_API UBTTask_Retreat : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Retreat();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float SafeDistance = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float RetreatSpeed = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptanceRadius = 100.0f;
};
