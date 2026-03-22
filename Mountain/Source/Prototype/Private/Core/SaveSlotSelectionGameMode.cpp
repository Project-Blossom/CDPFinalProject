#include "Core/SaveSlotSelectionGameMode.h"
#include "UI/SaveSlotSelectionWidget.h"
#include "Kismet/GameplayStatics.h"

ASaveSlotSelectionGameMode::ASaveSlotSelectionGameMode()
{
    // DefaultPawn 없음
    DefaultPawnClass = nullptr;
}

void ASaveSlotSelectionGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Widget 생성 및 표시
    if (SaveSlotWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            SaveSlotWidgetInstance = CreateWidget<USaveSlotSelectionWidget>(PC, SaveSlotWidgetClass);
            if (SaveSlotWidgetInstance)
            {
                SaveSlotWidgetInstance->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("Save Slot Selection displayed"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSlotWidgetClass not set in SaveSlotSelectionGameMode!"));
    }
}
