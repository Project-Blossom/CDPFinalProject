#include "AI/Tasks/BTTask_MoveToPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/FlyingMonster.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

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
    
    // [DEBUG] 추적 상태 시각화
#if !UE_BUILD_SHIPPING
    // 1. FlyingPlatform -> Player 추적선 (굵은 빨간선)
    DrawDebugLine(
        Monster->GetWorld(),
        Monster->GetActorLocation(),
        PlayerLocation,
        FColor::Red,
        false,
        0.1f,
        0,
        5.0f
    );
    
    // 2. 목표 지점 구체 (노란색)
    DrawDebugSphere(
        Monster->GetWorld(),
        PlayerLocation,
        AcceptanceRadius,
        16,
        FColor::Yellow,
        false,
        0.1f,
        0,
        2.0f
    );
    
    // 3. 화면 상단에 추적 정보 표시
    if (GEngine)
    {
        FString DebugMsg = FString::Printf(
            TEXT("[TRACKING PLAYER] %s -> Player | Distance: %.0f cm | Target: %.0f cm"),
            *Monster->GetName(),
            Distance,
            AcceptanceRadius
        );
        
        GEngine->AddOnScreenDebugMessage(
            -1,
            0.1f,
            Distance <= AcceptanceRadius ? FColor::Green : FColor::Orange,
            DebugMsg
        );
    }
#endif
    
    if (Distance <= AcceptanceRadius)
    {
        // 도착!
        UE_LOG(LogTemp, Warning, TEXT("%s ARRIVED at player! Distance: %.1f cm"), *Monster->GetName(), Distance);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // FlyingMonster의 FlyToLocation 사용
    Monster->FlyToLocation(PlayerLocation, MovementSpeed, true);
}
