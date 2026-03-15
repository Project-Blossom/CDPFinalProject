#include "AI/Services/BTService_CheckPlayerDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "DownfallCharacter.h"

UBTService_CheckPlayerDistance::UBTService_CheckPlayerDistance()
{
    NodeName = "Check Player Distance";
    
    // 0.2초마다 체크
    Interval = 0.2f;
    RandomDeviation = 0.05f;
}

void UBTService_CheckPlayerDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
        return;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
        return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
        return;

    // TargetPlayer 가져오기
    UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName);
    ADownfallCharacter* TargetPlayer = Cast<ADownfallCharacter>(TargetObject);

    bool bIsNearby = false;

    if (TargetPlayer)
    {
        // 거리 계산
        float Distance = FVector::Dist(AIPawn->GetActorLocation(), TargetPlayer->GetActorLocation());
        
        // 가까운지 체크
        bIsNearby = (Distance <= NearbyDistance);
        
        // 디버그 로그
        UE_LOG(LogTemp, Verbose, TEXT("CheckPlayerDistance: Distance=%.1f, Nearby=%s (threshold=%.1f)"), 
            Distance, bIsNearby ? TEXT("YES") : TEXT("NO"), NearbyDistance);
    }

    // Blackboard에 저장
    BlackboardComp->SetValueAsBool(bPlayerNearbyKey.SelectedKeyName, bIsNearby);
}
