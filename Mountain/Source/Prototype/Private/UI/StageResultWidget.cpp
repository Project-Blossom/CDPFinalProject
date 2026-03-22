#include "UI/StageResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DownfallGameInstance.h"

void UStageResultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (RetryButton)
    {
        RetryButton->OnClicked.AddDynamic(this, &UStageResultWidget::HandleRetryClicked);
    }

    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UStageResultWidget::HandleMainMenuClicked);
    }

    if (NextStageButton)
    {
        NextStageButton->OnClicked.AddDynamic(this, &UStageResultWidget::HandleNextStageClicked);
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

void UStageResultWidget::SetStageInfo(FName InStageId, float InLastTime, float InBestTime, int32 InClearCount)
{
    CurrentStageId = InStageId;

    // 스테이지 이름
    if (StageNameText)
    {
        StageNameText->SetText(FText::FromName(InStageId));
    }

    // Last Time
    if (LastTimeText)
    {
        FString TimeStr = FString::Printf(TEXT("Last Time: %.2f seconds"), InLastTime);
        LastTimeText->SetText(FText::FromString(TimeStr));
    }

    // Best Time
    if (BestTimeText)
    {
        FString BestStr = FString::Printf(TEXT("Best Time: %.2f seconds"), InBestTime);
        BestTimeText->SetText(FText::FromString(BestStr));
    }

    // Clear Count
    if (ClearCountText)
    {
        FString CountStr = FString::Printf(TEXT("Clear Count: %d"), InClearCount);
        ClearCountText->SetText(FText::FromString(CountStr));
    }
}

void UStageResultWidget::OnRetryClicked()
{
    HandleRetryClicked();
}

void UStageResultWidget::OnMainMenuClicked()
{
    HandleMainMenuClicked();
}

void UStageResultWidget::OnNextStageClicked()
{
    HandleNextStageClicked();
}

void UStageResultWidget::HandleRetryClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Retry clicked - Reloading stage: %s"), *CurrentStageId.ToString());

    // 현재 레벨 재시작
    UGameplayStatics::OpenLevel(this, CurrentStageId);
}

void UStageResultWidget::HandleMainMenuClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Main Menu clicked - Returning to Main Menu"));

    // 메인 메뉴 레벨로 이동
    UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UStageResultWidget::HandleNextStageClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Next Stage clicked"));

    // TODO: 다음 스테이지로 이동 (스테이지 목록 시스템이 만들어지면)
    // FName NextStageName = GetNextStageName(CurrentStageId);
    // UGameplayStatics::OpenLevel(this, NextStageName);
    
    UE_LOG(LogTemp, Warning, TEXT("Next Stage not implemented yet"));
}
