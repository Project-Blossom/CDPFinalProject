// File: Source/Prototype/Public/UI/CliffSelectionLoadingWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CliffSelectionLoadingWidget.generated.h"

/**
 * CliffSelection 레벨 로딩 UI (WBP_CliffSelectionLoading)
 * - 암벽 3개 생성 완료 전까지 표시
 * - ACliffSelectionGameMode::OnAllCliffsGenerated 수신 시 자동으로 화면에서 제거
 */
UCLASS()
class PROTOTYPE_API UCliffSelectionLoadingWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    // GameMode의 OnAllCliffsGenerated 델리게이트 수신
    UFUNCTION()
    void HandleAllCliffsGenerated();

    bool bIsBound = false;
};
