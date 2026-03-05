#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPatrolLocation.generated.h"

/**
 * Task: 랜덤 배회 위치를 찾아서 Blackboard에 저장
 */
UCLASS()
class PROTOTYPE_API UBTTask_FindPatrolLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FindPatrolLocation();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolLocationKey;
    
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float VerticalRange = 500.0f;
};
