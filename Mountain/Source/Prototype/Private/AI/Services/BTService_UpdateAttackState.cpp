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

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
        return;

    AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
    if (!Monster)
        return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
        return;

    BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Cast<UObject>(Monster->TargetPlayer));

    float LastAttackTime = BlackboardComp->GetValueAsFloat(LastAttackTimeKey.SelectedKeyName);
    float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
    float TimeSinceAttack = CurrentTime - LastAttackTime;
    
    bool bCanAttack = TimeSinceAttack >= AttackCooldown;
    BlackboardComp->SetValueAsBool(bCanAttackKey.SelectedKeyName, bCanAttack);
}
