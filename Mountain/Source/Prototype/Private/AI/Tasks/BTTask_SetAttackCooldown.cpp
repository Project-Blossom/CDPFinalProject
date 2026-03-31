#include "AI/Tasks/BTTask_SetAttackCooldown.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_SetAttackCooldown::UBTTask_SetAttackCooldown()
{
    NodeName = "Set Attack Cooldown";
    
    bNotifyTick = false;
    bNotifyTaskFinished = false;
}

EBTNodeResult::Type UBTTask_SetAttackCooldown::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_SetAttackCooldown: No Blackboard!"));
        return EBTNodeResult::Failed;
    }

    // 현재 시간을 LastAttackTime에 저장
    float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
    BlackboardComp->SetValueAsFloat(LastAttackTimeKey.SelectedKeyName, CurrentTime);
    
    // 즉시 bCanAttack을 false로 설정
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool("bCanAttack", false);
    }
    
    //UE_LOG(LogTemp, Warning, TEXT("BTTask_SetAttackCooldown: LastAttackTime set to %.2f, bCanAttack set to FALSE"), CurrentTime);
    
    return EBTNodeResult::Succeeded;
}
