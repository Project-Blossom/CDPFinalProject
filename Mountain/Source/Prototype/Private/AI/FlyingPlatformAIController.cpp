#include "AI/FlyingPlatformAIController.h"
#include "Monsters/FlyingPlatform.h"

AFlyingPlatformAIController::AFlyingPlatformAIController()
{
    UE_LOG(LogTemp, Log, TEXT("FlyingPlatformAIController created"));
}

void AFlyingPlatformAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AFlyingPlatform* FlyingPlatform = Cast<AFlyingPlatform>(InPawn);
    if (FlyingPlatform)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyingPlatformAIController possessed FlyingPlatform: %s"), *FlyingPlatform->GetName());
    }
}
