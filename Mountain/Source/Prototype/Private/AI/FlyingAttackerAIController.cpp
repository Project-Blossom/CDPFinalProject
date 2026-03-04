#include "AI/FlyingAttackerAIController.h"
#include "Monsters/FlyingAttacker.h"

AFlyingAttackerAIController::AFlyingAttackerAIController()
{
    UE_LOG(LogTemp, Log, TEXT("FlyingAttackerAIController created"));
}

void AFlyingAttackerAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AFlyingAttacker* FlyingAttacker = Cast<AFlyingAttacker>(InPawn);
    if (FlyingAttacker)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyingAttackerAIController possessed FlyingAttacker: %s"), *FlyingAttacker->GetName());
    }
}
