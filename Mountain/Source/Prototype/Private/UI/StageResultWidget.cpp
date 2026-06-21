#include "UI/StageResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DownfallGameInstance.h"
#include "UI/UIButtonClickSoundHelper.h"

void UStageResultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 공통 버튼 클릭음 적용
    PrototypeUI::ApplyProjectButtonClickSound(this);

    // 결과 화면에 진입하면 전용 BGM으로 교체한다.
    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->PlayMenuBGM(this, GI->StageResultBGM);
    }

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

    // 현재 레벨 재시작: 실제 플레이 스테이지 진입 전 결과 BGM을 종료한다.
    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->StopMenuBGM(0.0f);
    }

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

    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(
        UGameplayStatics::GetGameInstance(this));

    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("StageResultWidget: GameInstance not found"));
        return;
    }

    const int32 CurrentStageIndex = GI->GetCurrentStageIndex();

    // Stage_3 이후는 Ending 미구현 — 현재는 메인 메뉴로
    if (CurrentStageIndex >= 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("StageResult: All stages cleared, returning to Main Menu (Ending TBD)"));
        UGameplayStatics::OpenLevel(this, FName("MainMenu"));
        return;
    }

    // 다음은 CliffSelection 레벨 → 여기서 암벽 선택 후 다음 Stage로 진행
    UE_LOG(LogTemp, Warning, TEXT("StageResult: → CliffSelection [CurrentStageIndex=%d]"), CurrentStageIndex);

    // CliffSelectionPawn BeginPlay가 CliffSelectionBGM을 새로 재생한다.
    // 이전 Result BGM은 여기서 정리한다.
    GI->StopMenuBGM(0.0f);

    UGameplayStatics::OpenLevel(this, FName("CliffSelection"));
}