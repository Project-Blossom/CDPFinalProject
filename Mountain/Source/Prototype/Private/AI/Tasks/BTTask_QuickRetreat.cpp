#include "AI/Tasks/BTTask_QuickRetreat.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/FlyingMonster.h"

UBTTask_QuickRetreat::UBTTask_QuickRetreat()
{
    NodeName = "Quick Retreat";
    
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_QuickRetreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
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

    // 시작 위치와 시간 기록
    RetreatStartLocation = FlyingMonster->GetActorLocation();
    RetreatStartTime = OwnerComp.GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("BTTask_QuickRetreat: Starting quick retreat"));
    
    return EBTNodeResult::InProgress;
}

void UBTTask_QuickRetreat::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

    FVector CurrentLocation = FlyingMonster->GetActorLocation();
    float DistanceTraveled = FVector::Dist(CurrentLocation, RetreatStartLocation);
    float TimeElapsed = OwnerComp.GetWorld()->GetTimeSeconds() - RetreatStartTime;
    
    // 완료 조건: 목표 거리 도달 또는 시간 초과
    if (DistanceTraveled >= RetreatDistance || TimeElapsed >= MaxRetreatTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_QuickRetreat: Retreat complete (%.1fcm, %.2fs) - Ready for re-detection"), 
            DistanceTraveled, TimeElapsed);
        
        // 후퇴 완료 후 재감지를 위해 준비
        // Service가 다시 거리를 체크할 것임
        
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 후퇴 방향: 현재 전진 방향의 반대
    FVector ForwardDirection = FlyingMonster->GetActorForwardVector();
    FVector RetreatDirection = -ForwardDirection;
    
    // 목표 위치 계산
    FVector RetreatTarget = CurrentLocation + RetreatDirection * (RetreatDistance - DistanceTraveled);
    
    // 후퇴 (장애물 회피 ON)
    FlyingMonster->FlyToLocation(RetreatTarget, RetreatSpeed, true);
}
