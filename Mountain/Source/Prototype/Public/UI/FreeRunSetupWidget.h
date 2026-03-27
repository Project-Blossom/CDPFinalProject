#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FreeRunSetupWidget.generated.h"

/**
 * FreeRun 지형 생성 설정 UI
 */
UCLASS()
class PROTOTYPE_API UFreeRunSetupWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 버튼 클릭 핸들러
    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnEasyClicked();

    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnNormalClicked();

    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnHardClicked();

    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnRandomSeedClicked();

    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnGenerateClicked();

    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnStartClicked();

    UFUNCTION(BlueprintCallable, Category = "FreeRun Setup")
    void OnBackClicked();

protected:
    virtual void NativeConstruct() override;

    // UI 요소들
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* EasyButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* NormalButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* HardButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UEditableTextBox* SeedTextBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* RandomSeedButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* GenerateButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* StartButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* BackButton;

    // 선택된 Difficulty 표시
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedDifficultyText;

    // 레벨 설정
    UPROPERTY(EditDefaultsOnly, Category = "FreeRun Setup")
    FName MainMenuLevel = FName("MainMenu");

    UPROPERTY(EditDefaultsOnly, Category = "FreeRun Setup")
    FName FreeRunLevel = FName("FreeRun");

    // 미리보기 관련
    UPROPERTY()
    TObjectPtr<class AMountainGenWorldActor> PreviewMountain;

private:
    // 현재 선택된 Difficulty (0=Easy, 1=Normal, 2=Hard)
    int32 SelectedDifficulty = 1;

    // 버튼 핸들러
    UFUNCTION()
    void HandleEasyClicked();

    UFUNCTION()
    void HandleNormalClicked();

    UFUNCTION()
    void HandleHardClicked();

    UFUNCTION()
    void HandleRandomSeedClicked();

    UFUNCTION()
    void HandleGenerateClicked();

    UFUNCTION()
    void HandleStartClicked();

    UFUNCTION()
    void HandleBackClicked();

    UFUNCTION()
    void HandleSeedTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    // 미리보기 관련
    void FindPreviewMountain();
    void UpdatePreview(int32 Seed, int32 Difficulty);
    void UpdateSeedFromPreview();

    // UI 업데이트
    void UpdateDifficultyUI();

    // Timer 관련
    FTimerHandle SeedUpdateTimerHandle;
};
