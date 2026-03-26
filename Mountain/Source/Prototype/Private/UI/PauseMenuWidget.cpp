#include "UI/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DownfallGameMode.h"
#include "Core/DownfallGameInstance.h"

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleResumeClicked);
    }

    if (RetryButton)
    {
        RetryButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleRetryClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleSettingsClicked);
    }

    if (SaveAndQuitButton)
    {
        SaveAndQuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleSaveAndQuitClicked);
    }

    // Input Mode 설정
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

void UPauseMenuWidget::OnResumeClicked()
{
    HandleResumeClicked();
}

void UPauseMenuWidget::OnRetryClicked()
{
    HandleRetryClicked();
}

void UPauseMenuWidget::OnSettingsClicked()
{
    HandleSettingsClicked();
}

void UPauseMenuWidget::OnSaveAndQuitClicked()
{
    HandleSaveAndQuitClicked();
}

void UPauseMenuWidget::HandleResumeClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Resume clicked - Unpausing game"));

    // 게임 일시정지 해제
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // Input Mode 복원
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    // Widget 제거
    RemoveFromParent();
}

void UPauseMenuWidget::HandleRetryClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Retry clicked - Restarting stage"));

    // 일시정지 해제
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // 현재 레벨 재시작
    FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld()));
    UGameplayStatics::OpenLevel(this, CurrentLevel);
}

void UPauseMenuWidget::HandleSettingsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Settings clicked - Not implemented yet"));

    // TODO: Settings 화면 열기
    // 일단 아무 동작 안 함
}

void UPauseMenuWidget::HandleSaveAndQuitClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Save & Quit clicked - Saving and returning to main menu"));

    // GameMode에서 현재 경과 시간 가져오기
    ADownfallGameMode* GameMode = Cast<ADownfallGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode && GameMode->IsStageActive())
    {
        float ElapsedTime = GameMode->GetCurrentElapsedTime();
        FName StageId = GameMode->CurrentStageId;

        // GameInstance에 기록 저장
        UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
        if (GI)
        {
            GI->RecordStageTime(StageId, ElapsedTime);
            UE_LOG(LogTemp, Warning, TEXT("Stage progress saved: %s, Time: %.2f"), *StageId.ToString(), ElapsedTime);
        }
    }

    // 일시정지 해제
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // MainMenu로 이동
    UGameplayStatics::OpenLevel(this, MainMenuLevel);
}
