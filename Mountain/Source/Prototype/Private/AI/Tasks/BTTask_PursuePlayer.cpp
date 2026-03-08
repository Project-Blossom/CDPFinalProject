#include "AI/Tasks/BTTask_PursuePlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"

UBTTask_PursuePlayer::UBTTask_PursuePlayer()
{
    NodeName = "Pursue Player";
    bNotifyTick = true;

    TargetPlayerKey.SelectedKeyName = "TargetPlayer";
}

EBTNodeResult::Type UBTTask_PursuePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
        return EBTNodeResult::Failed;

    AWallCrawler* WallCrawler = Cast<AWallCrawler>(AIController->GetPawn());
    if (!WallCrawler)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
        return EBTNodeResult::Failed;

    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
    if (!TargetPlayer)
        return EBTNodeResult::Failed;

    UE_LOG(LogTemp, Log, TEXT("BTTask_PursuePlayer: START pursuing player"));

    return EBTNodeResult::InProgress;
}

void UBTTask_PursuePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
    if (!TargetPlayer)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // PursuePlayer 실행
    WallCrawler->PursuePlayer(DeltaSeconds);

    // AttachRange 내에 도달하면 성공
    float Distance = FVector::Dist(WallCrawler->GetActorLocation(), TargetPlayer->GetActorLocation());
    if (Distance <= AttachRange)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_PursuePlayer: COMPLETE - reached attach range"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
