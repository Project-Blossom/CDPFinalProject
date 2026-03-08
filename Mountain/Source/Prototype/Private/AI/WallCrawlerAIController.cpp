#include "AI/WallCrawlerAIController.h"
#include "Monsters/WallCrawler.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"

AWallCrawlerAIController::AWallCrawlerAIController()
{
    // BB_WallCrawler와 BT_WallCrawler를 로드
    static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("/Game/AI/Blackboards/BB_WallCrawler"));
    if (BBObject.Succeeded())
    {
        BlackboardAsset = BBObject.Object;
    }

    static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("/Game/AI/BehaviorTrees/BT_WallCrawler"));
    if (BTObject.Succeeded())
    {
        BehaviorTree = BTObject.Object;
    }

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
