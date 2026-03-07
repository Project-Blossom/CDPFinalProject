#include "AI/Tasks/BTTask_Retreat.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/FlyingMonster.h"
#include "DownfallCharacter.h"

UBTTask_Retreat::UBTTask_Retreat()
{
    NodeName = "Retreat";
    
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Retreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    AFlyingMonster* FlyingMonster = Cast<AFlyingMonster>(AIController->GetPawn());
    if (!FlyingMonster)
    {
        return EBTNodeResult::Failed;
    }

    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName);
    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(TargetObject);
    
    if (!TargetPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Retreat: No target player"));
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("BTTask_Retreat: Retreating from player"));
    
    return EBTNodeResult::InProgress;
}

void UBTTask_Retreat::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    
    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName);
    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(TargetObject);
    
    if (!TargetPlayer)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector CurrentLocation = FlyingMonster->GetActorLocation();
    FVector PlayerLocation = TargetPlayer->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, PlayerLocation);
    
    if (Distance >= SafeDistance - AcceptanceRadius)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_Retreat: Reached safe distance"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    FVector AwayDirection = (CurrentLocation - PlayerLocation).GetSafeNormal();
    FVector RetreatLocation = CurrentLocation + AwayDirection * 200.0f;
    
    FlyingMonster->FlyToLocation(RetreatLocation, RetreatSpeed, true);
}
