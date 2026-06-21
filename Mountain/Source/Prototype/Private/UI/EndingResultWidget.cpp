#include "UI/EndingResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DownfallGameInstance.h"
#include "UI/UIButtonClickSoundHelper.h"

void UEndingResultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 공통 버튼 클릭음 적용
    PrototypeUI::ApplyProjectButtonClickSound(this);

    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UEndingResultWidget::HandleMainMenuClicked);
    }

    // Input Mode 설정 (StageResultWidget/PauseMenuWidget과 동일한 패턴)
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    PopulateResults();

    if (FadeInAnim)
    {
        PlayAnimationForward(FadeInAnim);
    }
}

FText UEndingResultWidget::FormatClearTime(float Seconds)
{
    if (Seconds <= 0.0f)
    {
        return FText::FromString(TEXT("--:--"));
    }

    const int32 TotalSeconds = FMath::RoundToInt(Seconds);
    const int32 Minutes = TotalSeconds / 60;
    const int32 Secs = TotalSeconds % 60;

    return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Secs));
}

void UEndingResultWidget::SetStageRow(UTextBlock* TimeText, UTextBlock* NewText, UTextBlock* BestTimeText, FName StageId)
{
    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (!GI)
    {
        return;
    }

    const FStageTimeRecord ThisRun = GI->GetStageRecord(StageId);

    // [DEBUG] StageId 불일치 진단용. CurrentStageId(레벨 GameMode 기본값)와 여기 설정된
    // Stage1Id/Stage2Id/Stage3Id가 정확히 같은 문자열이어야 기록을 찾는다. ThisRun.bCleared가
    // 항상 false로 찍히면 십중팔구 StageId 문자열이 어긋난 것이다 — Output Log에서
    // "EndingResult: SetStageRow" 로 검색해서 실제 게임에 기록된 StageId들과 비교할 것.
    UE_LOG(LogTemp, Warning,
        TEXT("EndingResult: SetStageRow — Query=%s bCleared=%s LastTime=%.2f BestTime=%.2f (전체 기록 수=%d)"),
        *StageId.ToString(),
        ThisRun.bCleared ? TEXT("true") : TEXT("false"),
        ThisRun.LastTime, ThisRun.BestTime,
        GI->GetAllStageRecords().Num());

    for (const FStageTimeRecord& Record : GI->GetAllStageRecords())
    {
        UE_LOG(LogTemp, Warning, TEXT("EndingResult:   보유 기록 StageId=%s LastTime=%.2f BestTime=%.2f"),
            *Record.StageId.ToString(), Record.LastTime, Record.BestTime);
    }

    if (TimeText)
    {
        TimeText->SetText(FormatClearTime(ThisRun.LastTime));
    }

    // [NEW] 스테이지별 최고 기록(3개 SaveSlot 통틀어) — NEW 배지와 같은 데이터 소스를
    // 재사용한다. 한 번도 클리어한 적 없는 슬롯이 있어도 이 스테이지를 어느 슬롯에서든
    // 한 번이라도 클리어했다면 bHasOverallBest=true로 값이 채워진다.
    bool bHasOverallBest = false;
    const float OverallBest = GI->GetBestStageTimeAcrossAllSlots(StageId, bHasOverallBest);

    if (BestTimeText)
    {
        BestTimeText->SetText(bHasOverallBest
            ? FormatClearTime(OverallBest)
            : FText::FromString(TEXT("--:--")));
    }

    if (NewText)
    {
        // 이번 회차 기록이 3개 슬롯 통틀어 그 스테이지의 최고 기록과 같으면(=이번 회차가
        // 그 기록을 세웠으면) NEW로 표시한다. RecordStageTime()이 이미 SaveGame()까지
        // 끝낸 뒤(StageResult 화면을 거쳐 이 화면에 도달)이므로, 현재 슬롯의 최신 상태가
        // GetBestStageTimeAcrossAllSlots 계산에도 이미 반영돼 있다.
        const bool bIsNewRecord = ThisRun.bCleared && bHasOverallBest
            && FMath::IsNearlyEqual(ThisRun.LastTime, OverallBest, 0.05f);

        NewText->SetVisibility(bIsNewRecord ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UEndingResultWidget::PopulateResults()
{
    SetStageRow(Stage1TimeText, Stage1NewText, Stage1BestTimeText, Stage1Id);
    SetStageRow(Stage2TimeText, Stage2NewText, Stage2BestTimeText, Stage2Id);
    SetStageRow(Stage3TimeText, Stage3NewText, Stage3BestTimeText, Stage3Id);

    if (!OverallBestText)
    {
        return;
    }

    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (!GI)
    {
        return;
    }

    // 3개 SaveSlot 통틀어 "합산 클리어 타임"이 가장 짧은 슬롯을 찾는다.
    // GetSlotInfo()의 TotalPlayTime은 SaveToSlot()에서 클리어한 스테이지들의
    // BestTime 합으로 이미 계산돼 저장돼 있으므로 별도 계산 없이 그대로 재사용한다.
    float BestTotal = 0.0f;
    int32 BestSlotIndex = INDEX_NONE;

    for (int32 SlotIndex = 0; SlotIndex < 3; ++SlotIndex)
    {
        float SlotTotalPlayTime = 0.0f;
        int32 SlotClearedStages = 0;
        FDateTime SlotLastSaveTime;
        bool bSlotExists = false;

        GI->GetSlotInfo(SlotIndex, SlotTotalPlayTime, SlotClearedStages, SlotLastSaveTime, bSlotExists);

        // [DEBUG] OverallBestText 미표시 진단용 — 슬롯별로 실제 조회된 값을 그대로 찍는다.
        UE_LOG(LogTemp, Warning,
            TEXT("EndingResult: OverallBest 슬롯 조회 — Slot=%d Exists=%s ClearedStages=%d TotalPlayTime=%.2f"),
            SlotIndex, bSlotExists ? TEXT("true") : TEXT("false"), SlotClearedStages, SlotTotalPlayTime);

        if (bSlotExists && SlotClearedStages > 0
            && (BestSlotIndex == INDEX_NONE || SlotTotalPlayTime < BestTotal))
        {
            BestTotal = SlotTotalPlayTime;
            BestSlotIndex = SlotIndex;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("EndingResult: OverallBest 최종 — BestSlotIndex=%d BestTotal=%.2f"),
        BestSlotIndex, BestTotal);

    if (BestSlotIndex != INDEX_NONE)
    {
        OverallBestText->SetText(FText::FromString(FString::Printf(
            TEXT("전체 베스트 기록 — %s (Slot %d)"),
            *FormatClearTime(BestTotal).ToString(), BestSlotIndex + 1)));
    }
    else
    {
        OverallBestText->SetText(FText::FromString(TEXT("전체 베스트 기록 없음")));
    }
}

void UEndingResultWidget::OnMainMenuClicked()
{
    HandleMainMenuClicked();
}

void UEndingResultWidget::HandleMainMenuClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("EndingResult: Main Menu clicked - Returning to Main Menu"));

    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->StopMenuBGM(0.0f);
    }

    UGameplayStatics::OpenLevel(this, MainMenuLevel);
}
