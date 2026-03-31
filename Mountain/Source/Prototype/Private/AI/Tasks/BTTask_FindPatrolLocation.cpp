#include "AI/Tasks/BTTask_FindPatrolLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/FlyingAttacker.h"
#include "Monsters/FlyingMonster.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
    NodeName = "Find Patrol Location";
    
    bNotifyTick = false;
    bNotifyTaskFinished = false;
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        //UE_LOG(LogTemp, Error, TEXT("BTTask_FindPatrolLocation: No AIController"));
        return EBTNodeResult::Failed;
    }

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn)
    {
        //UE_LOG(LogTemp, Error, TEXT("BTTask_FindPatrolLocation: No Pawn"));
        return EBTNodeResult::Failed;
    }

    FVector PatrolLocation;
    
    // FlyingAttacker인 경우 Territory 시스템 사용
    AFlyingAttacker* Attacker = Cast<AFlyingAttacker>(Pawn);
    if (Attacker)
    {
        PatrolLocation = Attacker->GetRandomPatrolLocationInTerritory();
        //UE_LOG(LogTemp, Log, TEXT("BTTask_FindPatrolLocation: Using Territory system for %s"), 
            //*Pawn->GetName());
    }
    else
    {
        // 일반 Flying Monster (FlyingPlatform 등) - FlyingMonster의 메서드 사용
        AFlyingMonster* FlyingMonster = Cast<AFlyingMonster>(Pawn);
        if (FlyingMonster)
        {
            PatrolLocation = FlyingMonster->GetRandomPatrolLocation();
            //UE_LOG(LogTemp, Log, TEXT("BTTask_FindPatrolLocation: Using GetRandomPatrolLocation for %s"), 
                //*Pawn->GetName());
        }
        else
        {
            // FlyingMonster도 아닌 경우 기본 랜덤 생성
            FVector Origin = Pawn->GetActorLocation();
            FVector RandomOffset = FVector(
                FMath::RandRange(-PatrolRadius, PatrolRadius),
                FMath::RandRange(-PatrolRadius, PatrolRadius),
                FMath::RandRange(-VerticalRange * 0.5f, VerticalRange * 0.5f)
            );
            PatrolLocation = Origin + RandomOffset;
        }
    }

    // Blackboard에 저장
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, PatrolLocation);
        
        //UE_LOG(LogTemp, Log, TEXT("BTTask_FindPatrolLocation: Set patrol location to %s"), 
            //*PatrolLocation.ToString());
        
        return EBTNodeResult::Succeeded;
    }

    //UE_LOG(LogTemp, Error, TEXT("BTTask_FindPatrolLocation: No Blackboard"));
    return EBTNodeResult::Failed;
}
