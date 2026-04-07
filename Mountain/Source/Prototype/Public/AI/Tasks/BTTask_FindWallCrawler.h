#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindWallCrawler.generated.h"

/**
 * BT Task: FlyingPlatform이 근처 WallCrawler를 찾아서 부착
 */
UCLASS()
class PROTOTYPE_API UBTTask_FindWallCrawler : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FindWallCrawler();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
protected:
    // Blackboard Key: CarriedCrawler를 저장할 Key
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CarriedCrawlerKey;
    
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bHasCrawlerKey;
};
