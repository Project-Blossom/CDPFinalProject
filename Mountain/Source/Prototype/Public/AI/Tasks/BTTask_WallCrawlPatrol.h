#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WallCrawlPatrol.generated.h"

UCLASS()
class PROTOTYPE_API UBTTask_WallCrawlPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_WallCrawlPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolDuration = 5.0f;

    float ElapsedTime = 0.0f;
};
