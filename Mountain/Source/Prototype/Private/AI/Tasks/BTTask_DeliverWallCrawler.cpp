#include "AI/Tasks/BTTask_DeliverWallCrawler.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/WallCrawler.h"

UBTTask_DeliverWallCrawler::UBTTask_DeliverWallCrawler()
{
    NodeName = "Deliver WallCrawler";
    bNotifyTick = true;  // Tick 활성화
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

    UE_LOG(LogTemp, Log, TEXT("%s: DeliverWallCrawler - Waiting for drop position..."), *Platform->GetName());
    
    // InProgress 반환하여 TickTask에서 계속 체크
    return EBTNodeResult::InProgress;
}

void UBTTask_DeliverWallCrawler::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AFlyingPlatform* Platform = Cast<AFlyingPlatform>(AIController->GetPawn());
    if (!Platform)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // WallCrawler를 잃어버렸으면 실패
    if (!Platform->HasCarriedCrawler())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Lost WallCrawler during delivery"), *Platform->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 플레이어 근처인지 매 프레임 확인
    if (Platform->CanDropCrawler())
    {
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
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
