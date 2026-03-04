#pragma once

#include "CoreMinimal.h"
#include "AI/MonsterAIController.h"
#include "WallCrawlerAIController.generated.h"

/**
 * WallCrawler 전용 AI Controller
 */
UCLASS()
class PROTOTYPE_API AWallCrawlerAIController : public AMonsterAIController
{
    GENERATED_BODY()

public:
    AWallCrawlerAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
};
