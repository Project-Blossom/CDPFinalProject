#include "AI/Services/BTService_UpdateAttackState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Monsters/MonsterBase.h"
#include "DownfallCharacter.h"

UBTService_UpdateAttackState::UBTService_UpdateAttackState()
{
    NodeName = "Update Attack State";
    
    Interval = 0.5f;
    RandomDeviation = 0.1f;
}

void UBTService_UpdateAttackState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateAttackState::TickNode CALLED!"));

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAttackState: NO AIController!"));
        return;
    }

    AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
    if (!Monster)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAttackState: NO Monster!"));
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateAttackState: NO Blackboard!"));
        return;
    }

    // TargetPlayer 동기화
    BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Cast<UObject>(Monster->TargetPlayer));

    float LastAttackTime = BlackboardComp->GetValueAsFloat(LastAttackTimeKey.SelectedKeyName);
    float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
    float TimeSinceAttack = CurrentTime - LastAttackTime;
    
    bool bCanAttack = TimeSinceAttack >= AttackCooldown;
    BlackboardComp->SetValueAsBool(bCanAttackKey.SelectedKeyName, bCanAttack);
    
    // 디버그 로그
    UE_LOG(LogTemp, Log, TEXT("UpdateAttackState: TargetPlayer=%s, LastAttackTime=%.1f, Current=%.1f, TimeSince=%.1f, bCanAttack=%s"),
        Monster->TargetPlayer ? TEXT("YES") : TEXT("NO"),
        LastAttackTime,
        CurrentTime,
        TimeSinceAttack,
        bCanAttack ? TEXT("TRUE") : TEXT("FALSE"));
}
