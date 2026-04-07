#include "AI/Tasks/BTTask_FindWallCrawler.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/WallCrawler.h"
#include "DrawDebugHelpers.h"

UBTTask_FindWallCrawler::UBTTask_FindWallCrawler()
{
    NodeName = "Find WallCrawler";
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_FindWallCrawler::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    AFlyingPlatform* Platform = Cast<AFlyingPlatform>(AIController->GetPawn());
    if (!Platform)
    {
        return EBTNodeResult::Failed;
    }

    // 이미 WallCrawler를 태우고 있으면 스킵
    if (Platform->HasCarriedCrawler())
    {
        UE_LOG(LogTemp, Log, TEXT("%s: Already has a WallCrawler"), *Platform->GetName());
        return EBTNodeResult::Succeeded;
    }

    // 마지막 탐색으로부터 5초 경과 확인
    float CurrentTime = Platform->GetWorld()->GetTimeSeconds();
    if (CurrentTime - Platform->LastSearchTime < Platform->CrawlerSearchInterval)
    {
        return EBTNodeResult::Failed;
    }

    Platform->LastSearchTime = CurrentTime;

#if !UE_BUILD_SHIPPING
    // 디버그 시각화: WallCrawler 탐지 범위 (Yellow)
    DrawDebugSphere(
        Platform->GetWorld(), 
        Platform->GetActorLocation(), 
        Platform->WallCrawlerDetectionRadius, 
        12, 
        FColor::Yellow, 
        false, 
        2.0f, 
        0, 
        3.0f
    );
#endif

    // WallCrawler 찾기
    AWallCrawler* FoundCrawler = Platform->FindNearbyWallCrawler();
    
    if (FoundCrawler)
    {
        // Blackboard에 저장
        UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
        if (Blackboard)
        {
            Blackboard->SetValueAsObject(CarriedCrawlerKey.SelectedKeyName, FoundCrawler);
            Blackboard->SetValueAsBool(bHasCrawlerKey.SelectedKeyName, true);
        }

        UE_LOG(LogTemp, Log, TEXT("%s: Found and attached WallCrawler: %s"), 
            *Platform->GetName(), *FoundCrawler->GetName());
        
        return EBTNodeResult::Succeeded;
    }

    UE_LOG(LogTemp, Log, TEXT("%s: No WallCrawler found nearby"), *Platform->GetName());
    return EBTNodeResult::Failed;
}
