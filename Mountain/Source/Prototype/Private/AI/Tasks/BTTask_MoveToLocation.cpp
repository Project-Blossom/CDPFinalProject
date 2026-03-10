#include "AI/Tasks/BTTask_MoveToLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Monsters/FlyingMonster.h"

UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
    NodeName = "Move To Location";
    
    // Tick 활성화
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_MoveToLocation: No Blackboard"));
        return EBTNodeResult::Failed;
    }

    // 목표 위치 가져오기
    FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
    
    if (TargetLocation.IsNearlyZero())
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_MoveToLocation: Target location is zero"));
        return EBTNodeResult::Failed;
    }

    // FlyingMonster의 장애물 기억 초기화 (새 Task 시작)
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        AFlyingMonster* FlyingMonster = Cast<AFlyingMonster>(AIController->GetPawn());
        if (FlyingMonster && FlyingMonster->bHasObstacleMemory)
        {
            UE_LOG(LogTemp, Log, TEXT("BTTask_MoveToLocation: Clearing obstacle memory for new target"));
            FlyingMonster->bHasObstacleMemory = false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BTTask_MoveToLocation: Moving to %s"), *TargetLocation.ToString());
    
    // InProgress를 반환하면 TickTask가 계속 호출됨
    return EBTNodeResult::InProgress;
}

void UBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
    
    // 현재 위치
    FVector CurrentLocation = Pawn->GetActorLocation();
    
    // 목표까지 거리
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    // 도착 체크
    if (Distance <= AcceptanceRadius)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_MoveToLocation: Arrived at target"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // FlyingMonster인 경우 FlyToLocation 사용 (장애물 회피 포함)
    AFlyingMonster* FlyingMonster = Cast<AFlyingMonster>(Pawn);
    if (FlyingMonster)
    {
        // 장애물 기억이 있으면 Task 실패 (목표 재설정 트리거)
        if (FlyingMonster->bHasObstacleMemory)
        {
            UE_LOG(LogTemp, Warning, TEXT("BTTask_MoveToLocation: Obstacle detected! Task FAILED"));
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // FlyToLocation 사용 (장애물 회피 ON)
        FlyingMonster->FlyToLocation(TargetLocation, MoveSpeed, true);
    }
    else
    {
        // 일반 Pawn은 기존 방식
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * MoveSpeed * DeltaSeconds;
        
        Pawn->SetActorLocation(NewLocation);
        
        // 목표 방향으로 회전
        if (!Direction.IsNearlyZero())
        {
            FRotator TargetRotation = Direction.Rotation();
            FRotator NewRotation = FMath::RInterpTo(Pawn->GetActorRotation(), TargetRotation, DeltaSeconds, 5.0f);
            Pawn->SetActorRotation(NewRotation);
        }
    }
}
