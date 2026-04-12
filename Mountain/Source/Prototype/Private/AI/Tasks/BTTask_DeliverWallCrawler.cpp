#include "AI/Tasks/BTTask_DeliverWallCrawler.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"
#include "Kismet/GameplayStatics.h"

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
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("%s: Starting WallCrawler delivery"), *Platform->GetName());
    
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
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 플레이어 위치 가져오기
    ADownfallCharacter* Player = Cast<ADownfallCharacter>(UGameplayStatics::GetPlayerCharacter(Platform->GetWorld(), 0));
    if (Player)
    {
        // 플레이어 위로 이동
        FVector PlayerLocation = Player->GetActorLocation();
        FVector TargetLocation = PlayerLocation + FVector(0, 0, 500.0f); // 플레이어 위 5m
        
        Platform->FlyToLocation(TargetLocation, Platform->FlightSpeed, false);
    }

    // 플레이어 근처인지 체크
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

        UE_LOG(LogTemp, Log, TEXT("%s: WallCrawler delivered successfully"), *Platform->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
