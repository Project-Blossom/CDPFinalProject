#include "AI/Tasks/BTTask_DeliverWallCrawler.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/WallCrawler.h"

UBTTask_DeliverWallCrawler::UBTTask_DeliverWallCrawler()
{
    NodeName = "Deliver WallCrawler";
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_DeliverWallCrawler::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // WallCrawler를 태우고 있지 않으면 실패
    if (!Platform->HasCarriedCrawler())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: DeliverWallCrawler - No crawler to deliver"), *Platform->GetName());
        return EBTNodeResult::Failed;
    }

    // 플레이어 근처인지 확인
    if (!Platform->CanDropCrawler())
    {
        // 아직 플레이어 근처가 아님
        return EBTNodeResult::Failed;
    }

    // WallCrawler 떨어뜨리기
    Platform->DropWallCrawler();

    // Blackboard 업데이트
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Blackboard)
    {
        Blackboard->ClearValue(CarriedCrawlerKey.SelectedKeyName);
        Blackboard->SetValueAsBool(bHasCrawlerKey.SelectedKeyName, false);
    }

    UE_LOG(LogTemp, Log, TEXT("%s: Successfully delivered WallCrawler to player"), *Platform->GetName());
    return EBTNodeResult::Succeeded;
}
