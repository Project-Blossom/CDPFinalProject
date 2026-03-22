#include "UI/OverwriteWarningWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DownfallGameInstance.h"

void UOverwriteWarningWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UOverwriteWarningWidget::HandleConfirmClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UOverwriteWarningWidget::HandleCancelClicked);
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

void UOverwriteWarningWidget::SetSlotIndex(int32 InSlotIndex)
{
    SlotIndex = InSlotIndex;

    // 경고 메시지 업데이트
    if (WarningText)
    {
        FString WarningMsg = FString::Printf(
            TEXT("슬롯 %d의 기존 데이터가 삭제됩니다.\n계속하시겠습니까?"),
            SlotIndex + 1
        );
        WarningText->SetText(FText::FromString(WarningMsg));
    }
}

void UOverwriteWarningWidget::OnConfirmClicked()
{
    HandleConfirmClicked();
}

void UOverwriteWarningWidget::OnCancelClicked()
{
    HandleCancelClicked();
}

void UOverwriteWarningWidget::HandleConfirmClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Overwrite confirmed for slot %d"), SlotIndex);

    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
    if (GI && SlotIndex >= 0 && SlotIndex <= 2)
    {
        // 슬롯 삭제 및 설정
        GI->DeleteSlot(SlotIndex);
        GI->SetCurrentSaveSlot(SlotIndex);

        // 첫 스테이지로 이동
        UGameplayStatics::OpenLevel(this, FirstStageLevel);
    }
}

void UOverwriteWarningWidget::HandleCancelClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Overwrite cancelled"));

    // 다이얼로그 닫기
    RemoveFromParent();

    // Input Mode 복원
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}
