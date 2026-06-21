// File: Source/Prototype/Private/UI/CliffSelectionHUDWidget.cpp
#include "UI/CliffSelectionHUDWidget.h"
#include "Core/CliffSelectionGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UIButtonClickSoundHelper.h"

void UCliffSelectionHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 리롤 버튼은 공통 클릭음 대신 전용 Reroll Sound만 재생한다.
    PrototypeUI::ApplyProjectButtonClickSound(this, RerollButton);

    if (RerollButton)
    {
        RerollButton->OnClicked.AddDynamic(this, &UCliffSelectionHUDWidget::HandleRerollClicked);

        // GameMode에서 이미 리롤을 사용한 상태라면(예: 위젯 재생성) 버튼 비활성화 동기화
        if (ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            if (GM->HasUsedReroll())
            {
                SetRerollButtonUsed();
            }
        }
    }
}

void UCliffSelectionHUDWidget::UpdateSelectionInfo(int32 CliffIndex, int32 Seed, const FString& InDifficultyText)
{
    if (SeedText)
    {
        SeedText->SetText(FText::FromString(FString::Printf(TEXT("Seed: %d"), Seed)));
    }

    if (DifficultyText)
    {
        DifficultyText->SetText(FText::FromString(InDifficultyText));
    }
}

void UCliffSelectionHUDWidget::SetInputHintVisible(bool bVisible)
{
    if (InputHintText)
    {
        InputHintText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UCliffSelectionHUDWidget::HandleRerollClicked()
{
    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM)
    {
        return;
    }

    if (GM->HasUsedReroll())
    {
        return;
    }

    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->PlayUISound(this, GI->CliffSelectionRerollSound);
    }

    GM->RerollCliffs();
    SetRerollButtonUsed();
}

void UCliffSelectionHUDWidget::SetRerollButtonUsed()
{
    if (RerollButton)
    {
        RerollButton->SetIsEnabled(false);

        // 시각적 비활성화 처리 (회색조)
        FLinearColor DisabledColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.f);
        RerollButton->SetColorAndOpacity(DisabledColor);
    }
}