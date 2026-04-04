#include "AI/Tasks/BTTask_MoveToPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/FlyingMonster.h"
#include "GameFramework/Character.h"

UBTTask_MoveToPlayer::UBTTask_MoveToPlayer()
{
    NodeName = "Move To Player";
    bNotifyTick = true;
    
    TargetPlayerKey.SelectedKeyName = "TargetPlayer";
}

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetPlayer = Cast<AActor>(Blackboard->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
    if (!TargetPlayer)
    {
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_MoveToPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AFlyingMonster* Monster = Cast<AFlyingMonster>(AIController->GetPawn());
    if (!Monster)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* TargetPlayer = Cast<AActor>(Blackboard->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
    if (!TargetPlayer)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector PlayerLocation = TargetPlayer->GetActorLocation();
    float Distance = FVector::Dist(Monster->GetActorLocation(), PlayerLocation);
    
    if (Distance <= AcceptanceRadius)
    {
        // 도착!
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // FlyingMonster의 FlyToLocation 사용
    Monster->FlyToLocation(PlayerLocation, MovementSpeed, true);
}
