#include "AI/Tasks/BTTask_WallCrawlPatrol.h"
#include "AIController.h"
#include "Monsters/WallCrawler.h"

UBTTask_WallCrawlPatrol::UBTTask_WallCrawlPatrol()
{
    NodeName = "Wall Crawl Patrol";
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_WallCrawlPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
        return EBTNodeResult::Failed;

    AWallCrawler* WallCrawler = Cast<AWallCrawler>(AIController->GetPawn());
    if (!WallCrawler)
        return EBTNodeResult::Failed;

    ElapsedTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("BTTask_WallCrawlPatrol: START patrol"));

    return EBTNodeResult::InProgress;
}

void UBTTask_WallCrawlPatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AWallCrawler* WallCrawler = Cast<AWallCrawler>(AIController->GetPawn());
    if (!WallCrawler)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // OrganicPatrol 실행
    WallCrawler->OrganicPatrol(DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    // PatrolDuration 동안 순찰 후 성공
    if (ElapsedTime >= PatrolDuration)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_WallCrawlPatrol: COMPLETE - patrol finished"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
