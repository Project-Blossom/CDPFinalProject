#pragma once

#include "CoreMinimal.h"
#include "AI/MonsterAIController.h"
#include "FlyingPlatformAIController.generated.h"

/**
 * FlyingPlatform 전용 AI Controller
 */
UCLASS()
class PROTOTYPE_API AFlyingPlatformAIController : public AMonsterAIController
{
    GENERATED_BODY()

public:
    AFlyingPlatformAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
};
