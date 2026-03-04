#pragma once

#include "CoreMinimal.h"
#include "AI/MonsterAIController.h"
#include "FlyingAttackerAIController.generated.h"

/**
 * FlyingAttacker 전용 AI Controller
 */
UCLASS()
class PROTOTYPE_API AFlyingAttackerAIController : public AMonsterAIController
{
    GENERATED_BODY()

public:
    AFlyingAttackerAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
};
