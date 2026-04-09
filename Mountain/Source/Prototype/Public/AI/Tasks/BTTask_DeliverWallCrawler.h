#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DeliverWallCrawler.generated.h"

/**
 * BT Task: 플레이어 근처에서 WallCrawler를 떨어뜨림
 */
UCLASS()
class PROTOTYPE_API UBTTask_DeliverWallCrawler : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_DeliverWallCrawler();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    
protected:
    // Blackboard Key: CarriedCrawler
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CarriedCrawlerKey;
    
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bHasCrawlerKey;
};
