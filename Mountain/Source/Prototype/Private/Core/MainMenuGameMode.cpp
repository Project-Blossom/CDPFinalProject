#include "Core/MainMenuGameMode.h"
#include "UI/MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    // DefaultPawn 없음 (캐릭터 스폰 방지)
    DefaultPawnClass = nullptr;
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Widget 생성 및 표시
    if (MainMenuWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(PC, MainMenuWidgetClass);
            if (MainMenuWidgetInstance)
            {
                MainMenuWidgetInstance->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("Main Menu displayed"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MainMenuWidgetClass not set in MainMenuGameMode!"));
    }
}
