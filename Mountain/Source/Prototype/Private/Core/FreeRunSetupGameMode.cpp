#include "Core/FreeRunSetupGameMode.h"
#include "UI/FreeRunSetupWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "ShowFlags.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

    // PIE 에디터 보정: BeginPlay 직후 한 번 더 FreeRunSetup에만 Wireframe 강제
    if (GetWorld())
    {
        FTimerHandle TempHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TempHandle,
            []()
            {
                if (GEngine && GEngine->GameViewport)
                {
                    ApplyViewMode(EViewModeIndex::VMI_Wireframe, false, GEngine->GameViewport->EngineShowFlags);
                    GEngine->GameViewport->ViewModeIndex = EViewModeIndex::VMI_Wireframe;

                    UE_LOG(LogTemp, Warning, TEXT("FreeRunSetup - Wireframe reapplied from GameMode delayed"));
                }
            },
            0.05f,
            false
        );
    }
}