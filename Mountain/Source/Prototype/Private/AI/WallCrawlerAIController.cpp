#include "AI/WallCrawlerAIController.h"
#include "Monsters/WallCrawler.h"

AWallCrawlerAIController::AWallCrawlerAIController()
{
    UE_LOG(LogTemp, Log, TEXT("WallCrawlerAIController created"));
}

void AWallCrawlerAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AWallCrawler* WallCrawler = Cast<AWallCrawler>(InPawn);
    if (WallCrawler)
    {
        UE_LOG(LogTemp, Warning, TEXT("WallCrawlerAIController possessed WallCrawler: %s"), *WallCrawler->GetName());
    }
}
