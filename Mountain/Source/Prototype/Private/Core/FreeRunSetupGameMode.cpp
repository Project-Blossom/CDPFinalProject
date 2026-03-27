#include "Core/FreeRunSetupGameMode.h"
#include "UI/FreeRunSetupWidget.h"
#include "Kismet/GameplayStatics.h"

AFreeRunSetupGameMode::AFreeRunSetupGameMode()
{
    // DefaultPawn 없음 (캐릭터 스폰 방지)
    DefaultPawnClass = nullptr;
}

void AFreeRunSetupGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Widget 생성 및 표시
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    
    if (FreeRunSetupWidgetClass)
    {
        if (PC)
        {
            FreeRunSetupWidgetInstance = CreateWidget<UFreeRunSetupWidget>(PC, FreeRunSetupWidgetClass);
            if (FreeRunSetupWidgetInstance)
            {
                FreeRunSetupWidgetInstance->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("FreeRun Setup displayed"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FreeRunSetupWidgetClass not set in FreeRunSetupGameMode!"));
    }

    // ViewMode를 Wireframe으로 설정
    if (PC)
    {
        PC->ConsoleCommand(TEXT("viewmode wireframe"));
        UE_LOG(LogTemp, Warning, TEXT("FreeRunSetup - ViewMode set to Wireframe"));
    }
}
