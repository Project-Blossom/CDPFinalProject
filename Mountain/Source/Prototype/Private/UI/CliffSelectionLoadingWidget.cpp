// File: Source/Prototype/Private/UI/CliffSelectionLoadingWidget.cpp
#include "UI/CliffSelectionLoadingWidget.h"
#include "Core/CliffSelectionGameMode.h"
#include "Kismet/GameplayStatics.h"

void UCliffSelectionLoadingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        GM->OnAllCliffsGenerated.AddDynamic(this, &UCliffSelectionLoadingWidget::HandleAllCliffsGenerated);
        bIsBound = true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionLoadingWidget: ACliffSelectionGameMode not found"));
    }
}

void UCliffSelectionLoadingWidget::NativeDestruct()
{
    if (bIsBound)
    {
        if (ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GM->OnAllCliffsGenerated.RemoveDynamic(this, &UCliffSelectionLoadingWidget::HandleAllCliffsGenerated);
        }
        bIsBound = false;
    }

    Super::NativeDestruct();
}

void UCliffSelectionLoadingWidget::HandleAllCliffsGenerated()
{
    // 암벽 3개 생성 완료 -> 로딩 UI 제거
    RemoveFromParent();
}
