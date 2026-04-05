#include "AI/Services/BTService_UpdateDetection.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/MonsterBase.h"
#include "DownfallCharacter.h"

UBTService_UpdateDetection::UBTService_UpdateDetection()
{
    NodeName = "Update Detection";
    
    // 매 프레임마다 업데이트
    Interval = 0.0f;
    RandomDeviation = 0.0f;
}

void UBTService_UpdateDetection::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
        return;

    AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
    if (!Monster)
        return;

    // MonsterBase의 TargetPlayer와 DetectionGauge를 Blackboard에 동기화
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp)
    {
        // TargetPlayer 업데이트 (UObject*로 캐스팅)
        BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Cast<UObject>(Monster->TargetPlayer));
        
        // DetectionGauge 업데이트
        float CurrentGauge = Monster->GetDetectionGauge();
        BlackboardComp->SetValueAsFloat(DetectionGaugeKey.SelectedKeyName, CurrentGauge);
        
        // [DEBUG] DetectionGauge 로그
#if !UE_BUILD_SHIPPING
        if (CurrentGauge > 0.0f || Monster->TargetPlayer)
        {
            UE_LOG(LogTemp, Warning, TEXT("[UPDATE DETECTION] %s | DetectionGauge: %.1f | TargetPlayer: %s"), 
                *Monster->GetName(), 
                CurrentGauge,
                Monster->TargetPlayer ? TEXT("YES") : TEXT("NO"));
        }
#endif
    }
}