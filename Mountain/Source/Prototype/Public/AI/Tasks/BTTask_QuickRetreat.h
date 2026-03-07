#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_QuickRetreat.generated.h"

/**
 * Task: 공격 후 짧은 후퇴
 * 공격 방향의 반대로 빠르게 후진 (치고 빠지기)
 */
UCLASS()
class PROTOTYPE_API UBTTask_QuickRetreat : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_QuickRetreat();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float RetreatDistance = 300.0f;  // 후퇴 거리 (3m)
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float RetreatSpeed = 400.0f;  // 후퇴 속도
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxRetreatTime = 1.0f;  // 최대 후퇴 시간

private:
    FVector RetreatStartLocation;
    float RetreatStartTime;
};
