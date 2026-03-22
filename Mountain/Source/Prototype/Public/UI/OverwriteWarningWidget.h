#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverwriteWarningWidget.generated.h"

/**
 * 세이브 덮어쓰기 경고 다이얼로그
 */
UCLASS()
class PROTOTYPE_API UOverwriteWarningWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 슬롯 인덱스 설정
    UFUNCTION(BlueprintCallable, Category = "Overwrite Warning")
    void SetSlotIndex(int32 InSlotIndex);

    // 확인 버튼
    UFUNCTION(BlueprintCallable, Category = "Overwrite Warning")
    void OnConfirmClicked();

    // 취소 버튼
    UFUNCTION(BlueprintCallable, Category = "Overwrite Warning")
    void OnCancelClicked();

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* ConfirmButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* CancelButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* WarningText;

    // 첫 스테이지 레벨
    UPROPERTY(EditDefaultsOnly, Category = "Overwrite Warning")
    FName FirstStageLevel = FName("Stage1");

    // Fade Widget 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Overwrite Warning")
    TSubclassOf<class UFadeWidget> FadeWidgetClass;

    // Fade Out 시간
    UPROPERTY(EditDefaultsOnly, Category = "Overwrite Warning")
    float FadeOutDuration = 1.5f;

private:
    int32 SlotIndex = -1;

    UPROPERTY()
    TObjectPtr<class UFadeWidget> FadeWidgetInstance;

    FName PendingLevelName;

    void StartFadeOutToLevel(FName LevelName);
    void OnFadeOutComplete();

    UFUNCTION()
    void HandleConfirmClicked();

    UFUNCTION()
    void HandleCancelClicked();
};
