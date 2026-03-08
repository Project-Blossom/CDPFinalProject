#include "AI/Tasks/BTTask_SetAttachCooldown.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetAttachCooldown::UBTTask_SetAttachCooldown()
{
    NodeName = "Set Attach Cooldown";
    LastAttachTimeKey.SelectedKeyName = "LastAttachTime";
    bCanAttachKey.SelectedKeyName = "bCanAttach";
}

EBTNodeResult::Type UBTTask_SetAttachCooldown::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
        return EBTNodeResult::Failed;

    float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
    BlackboardComp->SetValueAsFloat(LastAttachTimeKey.SelectedKeyName, CurrentTime);
    BlackboardComp->SetValueAsBool(bCanAttachKey.SelectedKeyName, false);

    UE_LOG(LogTemp, Log, TEXT("BTTask_SetAttachCooldown: LastAttachTime set to %.2f, bCanAttach set to FALSE"), CurrentTime);

    return EBTNodeResult::Succeeded;
}
