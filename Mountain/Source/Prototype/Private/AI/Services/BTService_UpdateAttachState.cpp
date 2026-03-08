#include "AI/Services/BTService_UpdateAttachState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"

UBTService_UpdateAttachState::UBTService_UpdateAttachState()
{
    NodeName = "Update Attach State";
    
    Interval = 0.1f;
    RandomDeviation = 0.0f;

    TargetPlayerKey.SelectedKeyName = "TargetPlayer";
    LastAttachTimeKey.SelectedKeyName = "LastAttachTime";
    bCanAttachKey.SelectedKeyName = "bCanAttach";
}

void UBTService_UpdateAttachState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateAttachState::TickNode CALLED!"));

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAttachState: NO AIController!"));
        return;
    }

    AWallCrawler* WallCrawler = Cast<AWallCrawler>(AIController->GetPawn());
    if (!WallCrawler)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAttachState: NO WallCrawler!"));
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAttachState: NO Blackboard!"));
        return;
    }

    // TargetPlayer 동기화
    BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Cast<UObject>(WallCrawler->TargetPlayer));

    // 이미 붙어있으면 attach 불가
    if (WallCrawler->bAttachedToPlayer)
    {
        BlackboardComp->SetValueAsBool(bCanAttachKey.SelectedKeyName, false);
        return;
    }

    // Stunned 상태면 attach 불가
    if (WallCrawler->bIsStunned)
    {
        BlackboardComp->SetValueAsBool(bCanAttachKey.SelectedKeyName, false);
        return;
    }

    float LastAttachTime = BlackboardComp->GetValueAsFloat(LastAttachTimeKey.SelectedKeyName);
    float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
    float TimeSinceAttach = CurrentTime - LastAttachTime;
    
    bool bCanAttach = TimeSinceAttach >= AttachCooldown;
    BlackboardComp->SetValueAsBool(bCanAttachKey.SelectedKeyName, bCanAttach);
    
    UE_LOG(LogTemp, Log, TEXT("UpdateAttachState: TargetPlayer=%s, LastAttachTime=%.1f, Current=%.1f, TimeSince=%.1f, bCanAttach=%s, Attached=%s, Stunned=%s"),
        WallCrawler->TargetPlayer ? TEXT("YES") : TEXT("NO"),
        LastAttachTime,
        CurrentTime,
        TimeSinceAttach,
        bCanAttach ? TEXT("TRUE") : TEXT("FALSE"),
        WallCrawler->bAttachedToPlayer ? TEXT("YES") : TEXT("NO"),
        WallCrawler->bIsStunned ? TEXT("YES") : TEXT("NO"));
}
