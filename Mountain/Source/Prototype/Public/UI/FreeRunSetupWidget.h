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
    class UButton* BackButton;

    // 선택된 Difficulty 표시
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* SelectedDifficultyText;

    // 레벨 설정
    UPROPERTY(EditDefaultsOnly, Category = "FreeRun Setup")
    FName MainMenuLevel = FName("MainMenu");

    UPROPERTY(EditDefaultsOnly, Category = "FreeRun Setup")
    FName FreeRunLevel = FName("FreeRun");

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
    void HandleBackClicked();

    UFUNCTION()
    void HandleSeedTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    // UI 업데이트
    void UpdateDifficultyUI();
};
