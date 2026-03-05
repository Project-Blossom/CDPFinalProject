#include "AI/Tasks/BTTask_FindPatrolLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
    NodeName = "Find Patrol Location";
    
    // Task 속성 설정
    bNotifyTick = false;
    bNotifyTaskFinished = false;
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FindPatrolLocation: No AIController"));
        return EBTNodeResult::Failed;
    }

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_FindPatrolLocation: No Pawn"));
        return EBTNodeResult::Failed;
    }

    // 현재 위치 기준으로 랜덤 배회 위치 생성
    FVector Origin = Pawn->GetActorLocation();
    FVector RandomOffset = FVector(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-VerticalRange * 0.5f, VerticalRange * 0.5f)
    );
    FVector PatrolLocation = Origin + RandomOffset;

    // Blackboard에 저장
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, PatrolLocation);
        
        UE_LOG(LogTemp, Log, TEXT("BTTask_FindPatrolLocation: Set patrol location to %s"), 
            *PatrolLocation.ToString());
        
        return EBTNodeResult::Succeeded;
    }

    UE_LOG(LogTemp, Error, TEXT("BTTask_FindPatrolLocation: No Blackboard"));
    return EBTNodeResult::Failed;
}
