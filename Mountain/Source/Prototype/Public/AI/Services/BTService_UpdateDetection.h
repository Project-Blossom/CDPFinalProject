#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateDetection.generated.h"

/**
 * Service: MonsterBase의 감지 시스템을 업데이트하고 Blackboard에 반영
 */
UCLASS()
class PROTOTYPE_API UBTService_UpdateDetection : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateDetection();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;
    
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DetectionGaugeKey;
};
