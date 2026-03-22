#include "UI/SaveSlotSelectionWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DownfallGameInstance.h"
#include "UI/OverwriteWarningWidget.h"

void USaveSlotSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (Slot0Button)
    {
        Slot0Button->OnClicked.AddDynamic(this, &USaveSlotSelectionWidget::HandleSlot0Clicked);
    }

    if (Slot1Button)
    {
        Slot1Button->OnClicked.AddDynamic(this, &USaveSlotSelectionWidget::HandleSlot1Clicked);
    }

    if (Slot2Button)
    {
        Slot2Button->OnClicked.AddDynamic(this, &USaveSlotSelectionWidget::HandleSlot2Clicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &USaveSlotSelectionWidget::HandleBackClicked);
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

    // 슬롯 정보 업데이트
    UpdateSlotInfo();
}

void USaveSlotSelectionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

void USaveSlotSelectionWidget::UpdateSlotInfo()
{
    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
    if (!GI)
        return;

    bool bIsNewGame = GI->IsNewGameMode();

    // 각 슬롯 정보 업데이트
    for (int32 i = 0; i < 3; i++)
    {
        UTextBlock* InfoText = nullptr;
        UButton* SlotButton = nullptr;

        if (i == 0) { InfoText = Slot0InfoText; SlotButton = Slot0Button; }
        else if (i == 1) { InfoText = Slot1InfoText; SlotButton = Slot1Button; }
        else if (i == 2) { InfoText = Slot2InfoText; SlotButton = Slot2Button; }

        if (!InfoText || !SlotButton)
            continue;

        float PlayTime = 0.0f;
        int32 ClearedStages = 0;
        FDateTime LastSaveTime;
        bool bExists = false;

        GI->GetSlotInfo(i, PlayTime, ClearedStages, LastSaveTime, bExists);

        if (bExists)
        {
            // 시간 포맷 (시:분)
            int32 Hours = FMath::FloorToInt(PlayTime / 3600.0f);
            int32 Minutes = FMath::FloorToInt((PlayTime - Hours * 3600.0f) / 60.0f);

            // 날짜 포맷
            FString DateStr = FString::Printf(TEXT("%d/%d/%d"), 
                LastSaveTime.GetYear(), LastSaveTime.GetMonth(), LastSaveTime.GetDay());

            FString InfoStr = FString::Printf(
                TEXT("Slot %d\n플레이 시간: %dh %dm\n클리어: %d 스테이지\n마지막 저장: %s"),
                i + 1, Hours, Minutes, ClearedStages, *DateStr
            );

            InfoText->SetText(FText::FromString(InfoStr));

            // LoadGame 모드일 때만 활성화
            SlotButton->SetIsEnabled(true);
        }
        else
        {
            // 빈 슬롯
            FString InfoStr = FString::Printf(TEXT("Slot %d\n새 게임"), i + 1);
            InfoText->SetText(FText::FromString(InfoStr));

            // NewGame 모드일 때만 활성화
            SlotButton->SetIsEnabled(bIsNewGame);
        }
    }
}

void USaveSlotSelectionWidget::HandleSlotSelected(int32 SlotIndex)
{
    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
    if (!GI)
        return;

    bool bIsNewGame = GI->IsNewGameMode();
    bool bSlotExists = GI->DoesSaveSlotExist(SlotIndex);

    if (bIsNewGame)
    {
        // NewGame 모드
        if (bSlotExists)
        {
            // 기존 데이터 있음 → 경고 표시
            ShowConfirmDialog(SlotIndex);
        }
        else
        {
            // 빈 슬롯 → 바로 시작
            GI->DeleteSlot(SlotIndex);
            GI->SetCurrentSaveSlot(SlotIndex);
            UGameplayStatics::OpenLevel(this, FirstStageLevel);
        }
    }
    else
    {
        // LoadGame 모드
        if (bSlotExists)
        {
            // 데이터 로드 후 시작
            GI->LoadFromSlot(SlotIndex);
            
            // TODO: 마지막 플레이한 스테이지로 이동 (현재는 Stage1)
            UGameplayStatics::OpenLevel(this, FirstStageLevel);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot load empty slot %d"), SlotIndex);
        }
    }
}

void USaveSlotSelectionWidget::ShowConfirmDialog(int32 SlotIndex)
{
    if (!OverwriteWarningWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("OverwriteWarningWidgetClass not set!"));
        
        // Widget이 없으면 바로 진행
        UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
        if (GI)
        {
            GI->DeleteSlot(SlotIndex);
            GI->SetCurrentSaveSlot(SlotIndex);
            UGameplayStatics::OpenLevel(this, FirstStageLevel);
        }
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
        return;

    // 경고 Widget 생성
    CurrentWarningWidget = CreateWidget<UOverwriteWarningWidget>(PC, OverwriteWarningWidgetClass);
    if (CurrentWarningWidget)
    {
        CurrentWarningWidget->SetSlotIndex(SlotIndex);
        CurrentWarningWidget->AddToViewport(100); // 최상위에 표시
        
        UE_LOG(LogTemp, Warning, TEXT("Showing overwrite warning for slot %d"), SlotIndex);
    }
}

void USaveSlotSelectionWidget::OnSlot0Clicked()
{
    HandleSlot0Clicked();
}

void USaveSlotSelectionWidget::OnSlot1Clicked()
{
    HandleSlot1Clicked();
}

void USaveSlotSelectionWidget::OnSlot2Clicked()
{
    HandleSlot2Clicked();
}

void USaveSlotSelectionWidget::OnBackClicked()
{
    HandleBackClicked();
}

void USaveSlotSelectionWidget::HandleSlot0Clicked()
{
    HandleSlotSelected(0);
}

void USaveSlotSelectionWidget::HandleSlot1Clicked()
{
    HandleSlotSelected(1);
}

void USaveSlotSelectionWidget::HandleSlot2Clicked()
{
    HandleSlotSelected(2);
}

void USaveSlotSelectionWidget::HandleBackClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Back to Main Menu"));
    UGameplayStatics::OpenLevel(this, MainMenuLevel);
}
