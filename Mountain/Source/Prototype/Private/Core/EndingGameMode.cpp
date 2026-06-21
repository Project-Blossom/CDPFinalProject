// File: Source/Prototype/Private/Core/EndingGameMode.cpp
#include "Core/EndingGameMode.h"
#include "UI/EndingSequenceWidget.h"
#include "Kismet/GameplayStatics.h"

AEndingGameMode::AEndingGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEndingGameMode::BeginPlay()
{
    Super::BeginPlay();

    ShowEndingSequenceWidget();
}

void AEndingGameMode::ShowEndingSequenceWidget()
{
    if (IsValid(CurrentSequenceWidget))
    {
        // 이미 표시 중
        return;
    }

    if (!EndingSequenceWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EndingGameMode: EndingSequenceWidgetClass not set (BP Class Defaults에서 WBP_Ending 지정 필요)"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    CurrentSequenceWidget = CreateWidget<UEndingSequenceWidget>(PC, EndingSequenceWidgetClass);
    if (CurrentSequenceWidget)
    {
        CurrentSequenceWidget->AddToViewport();
    }
}
