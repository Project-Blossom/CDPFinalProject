#include "AI/Tasks/BTTask_ChargeAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "DownfallCharacter.h"

UBTTask_ChargeAttack::UBTTask_ChargeAttack()
{
    NodeName = "Charge Attack";
    
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_ChargeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }

    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName);
    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(TargetObject);
    
    if (!TargetPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ChargeAttack: No target player - FAILED"));
        return EBTNodeResult::Failed;
    }

    FVector AttackTarget = TargetPlayer->GetActorLocation();
    BlackboardComp->SetValueAsVector(AttackTargetLocationKey.SelectedKeyName, AttackTarget);
    
    ChargeStartTime = OwnerComp.GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("BTTask_ChargeAttack: START charging attack toward player"));
    
    return EBTNodeResult::InProgress;
}

void UBTTask_ChargeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName);
    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(TargetObject);
    
    float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
    float ChargeElapsed = CurrentTime - ChargeStartTime;
    
    if (ChargeElapsed >= ChargeTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ChargeAttack: Charge COMPLETE - SUCCEEDED"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    if (TargetPlayer)
    {
        FVector Direction = (TargetPlayer->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
        if (!Direction.IsNearlyZero())
        {
            FRotator TargetRotation = Direction.Rotation();
            FRotator NewRotation = FMath::RInterpTo(Pawn->GetActorRotation(), TargetRotation, DeltaSeconds, 10.0f);
            Pawn->SetActorRotation(NewRotation);
        }
    }
}
