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

    // Target 유효성 검사 (Vector 또는 Object)
    bool bHasValidTarget = false;
    
    if (BlackboardComp->IsVectorValueSet(TargetKey.SelectedKeyName))
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_FlyToTarget: Target is Vector (stored location)"));
        bHasValidTarget = true;
    }
    else
    {
        UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);
        if (TargetObject)
        {
            UE_LOG(LogTemp, Log, TEXT("BTTask_FlyToTarget: Target is Object (dynamic tracking)"));
            bHasValidTarget = true;
        }
    }
    
    if (!bHasValidTarget)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FlyToTarget: No valid target (neither Vector nor Object)"));
        return EBTNodeResult::Failed;
    }

    // Task 시작 시 충돌 플래그 초기화
    bHasHitPlayerThisTask = false;

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
    
    // Vector 타입인지 먼저 시도 (AttackTargetLocation - 저장된 위치)
    if (BlackboardComp->IsVectorValueSet(TargetKey.SelectedKeyName))
    {
        // Vector 키 - 저장된 위치 사용 (추적 X)
        TargetLocation = BlackboardComp->GetValueAsVector(TargetKey.SelectedKeyName);
        UE_LOG(LogTemp, Verbose, TEXT("BTTask_FlyToTarget: Using stored Vector target (no tracking)"));
    }
    else
    {
        // Object 키 시도 - Actor 위치 사용 (실시간 추적)
        UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);
        AActor* TargetActor = Cast<AActor>(TargetObject);
        
        if (TargetActor)
        {
            TargetLocation = TargetActor->GetActorLocation();
            UE_LOG(LogTemp, Verbose, TEXT("BTTask_FlyToTarget: Using Actor target (tracking)"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("BTTask_FlyToTarget: Invalid target key"));
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
    }

    FVector CurrentLocation = FlyingMonster->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    // 플레이어와의 충돌 체크 (FlyingAttacker용)
    if (!bHasHitPlayerThisTask)
    {
        // TargetPlayer를 Blackboard에서 직접 가져오기
        UObject* PlayerObject = BlackboardComp->GetValueAsObject("TargetPlayer");
        ADownfallCharacter* Player = Cast<ADownfallCharacter>(PlayerObject);
        
        if (Player)
        {
            float PlayerDistance = FVector::Dist(CurrentLocation, Player->GetActorLocation());
            if (PlayerDistance <= HitCheckRadius)
            {
                // 플레이어 타격!
                Player->AddInsanity(InsanityDamage);
                bHasHitPlayerThisTask = true;
                
                UE_LOG(LogTemp, Warning, TEXT("BTTask_FlyToTarget: HIT player! (+%.1f Insanity)"), InsanityDamage);
            }
        }
    }
    
    if (Distance <= AcceptanceRadius)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_FlyToTarget: Arrived at target"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    FlyingMonster->FlyToLocation(TargetLocation, FlightSpeed, bAvoidObstacles);
}
