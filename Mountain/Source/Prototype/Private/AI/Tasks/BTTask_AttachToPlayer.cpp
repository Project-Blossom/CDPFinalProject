#include "AI/Tasks/BTTask_AttachToPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"

UBTTask_AttachToPlayer::UBTTask_AttachToPlayer()
{
    NodeName = "Attach To Player";
    TargetPlayerKey.SelectedKeyName = "TargetPlayer";
}

EBTNodeResult::Type UBTTask_AttachToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    UE_LOG(LogTemp, Log, TEXT("BTTask_AttachToPlayer: Attaching to player"));

    WallCrawler->AttachToPlayer(TargetPlayer);

    return EBTNodeResult::Succeeded;
}
