#include "AI/Tasks/BTTask_FlyToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/FlyingMonster.h"
#include "DownfallCharacter.h"

UBTTask_FlyToTarget::UBTTask_FlyToTarget()
{
    NodeName = "Fly To Target";
    
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_FlyToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FlyToTarget: No Blackboard"));
        return EBTNodeResult::Failed;
    }

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FlyToTarget: No AIController"));
        return EBTNodeResult::Failed;
    }

    AFlyingMonster* FlyingMonster = Cast<AFlyingMonster>(AIController->GetPawn());
    if (!FlyingMonster)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FlyToTarget: Not a FlyingMonster"));
        return EBTNodeResult::Failed;
    }

    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);
    
    if (!TargetObject)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FlyToTarget: No target"));
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("BTTask_FlyToTarget: Flying to target"));
    
    return EBTNodeResult::InProgress;
}

void UBTTask_FlyToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AFlyingMonster* FlyingMonster = Cast<AFlyingMonster>(AIController->GetPawn());
    if (!FlyingMonster)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    FVector TargetLocation;
    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);
    
    if (TargetObject)
    {
        AActor* TargetActor = Cast<AActor>(TargetObject);
        if (TargetActor)
        {
            TargetLocation = TargetActor->GetActorLocation();
        }
        else
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
    }
    else
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector CurrentLocation = FlyingMonster->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    if (Distance <= AcceptanceRadius)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_FlyToTarget: Arrived at target"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    FlyingMonster->FlyToLocation(TargetLocation, FlightSpeed, bAvoidObstacles);
}
