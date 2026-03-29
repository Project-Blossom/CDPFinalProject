#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlotSelectionWidget.generated.h"

/**
 * 세이브 슬롯 선택 UI
 */
UCLASS()
class PROTOTYPE_API USaveSlotSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 슬롯 선택
    UFUNCTION(BlueprintCallable, Category = "Save Slot")
    void OnSlot0Clicked();

    UFUNCTION(BlueprintCallable, Category = "Save Slot")
    void OnSlot1Clicked();

    UFUNCTION(BlueprintCallable, Category = "Save Slot")
    void OnSlot2Clicked();

    // 뒤로 가기
    UFUNCTION(BlueprintCallable, Category = "Save Slot")
    void OnBackClicked();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // 슬롯 버튼들
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* Slot0Button;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* Slot1Button;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* Slot2Button;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* BackButton;

    // 슬롯 정보 텍스트
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* Slot0InfoText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* Slot1InfoText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* Slot2InfoText;

    // 메인 메뉴 레벨
    UPROPERTY(EditDefaultsOnly, Category = "Save Slot")
    FName MainMenuLevel = FName("MainMenu");

    // 첫 스테이지 레벨
    UPROPERTY(EditDefaultsOnly, Category = "Save Slot")
    FName FirstStageLevel = FName("Stage_1");

    // 덮어쓰기 경고 Widget 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Save Slot")
    TSubclassOf<class UOverwriteWarningWidget> OverwriteWarningWidgetClass;

    // Fade Widget 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Save Slot")
    TSubclassOf<class UFadeWidget> FadeWidgetClass;

    // Fade Out 후 레벨 전환 시간
    UPROPERTY(EditDefaultsOnly, Category = "Save Slot")
    float FadeOutDuration = 1.5f;

private:
    // 현재 표시 중인 경고 Widget
    UPROPERTY()
    TObjectPtr<class UOverwriteWarningWidget> CurrentWarningWidget;

    // Fade Widget Instance
    UPROPERTY()
    TObjectPtr<class UFadeWidget> FadeWidgetInstance;

    // Fade Out 후 전환할 레벨
    FName PendingLevelName;

    // Fade Out 시작
    void StartFadeOutToLevel(FName LevelName);

    // Fade Out 완료 후 레벨 전환
    void OnFadeOutComplete();
    // 슬롯 정보 업데이트
    void UpdateSlotInfo();

    // 슬롯 선택 처리
    void HandleSlotSelected(int32 SlotIndex);

    // 확인 다이얼로그 표시 (NewGame 덮어쓰기 경고)
    void ShowConfirmDialog(int32 SlotIndex);

    UFUNCTION()
    void HandleSlot0Clicked();

    UFUNCTION()
    void HandleSlot1Clicked();

    UFUNCTION()
    void HandleSlot2Clicked();

    UFUNCTION()
    void HandleBackClicked();
};
