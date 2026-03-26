#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

/**
 * 일시정지 메뉴 Widget
 */
UCLASS()
class PROTOTYPE_API UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 버튼 클릭 핸들러
    UFUNCTION(BlueprintCallable, Category = "Pause Menu")
    void OnResumeClicked();

    UFUNCTION(BlueprintCallable, Category = "Pause Menu")
    void OnRetryClicked();

    UFUNCTION(BlueprintCallable, Category = "Pause Menu")
    void OnSettingsClicked();

    UFUNCTION(BlueprintCallable, Category = "Pause Menu")
    void OnSaveAndQuitClicked();

protected:
    virtual void NativeConstruct() override;

    // UI 요소들
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* ResumeButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* RetryButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* SettingsButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* SaveAndQuitButton;

    // 레벨 설정
    UPROPERTY(EditDefaultsOnly, Category = "Pause Menu")
    FName MainMenuLevel = FName("MainMenu");

private:
    UFUNCTION()
    void HandleResumeClicked();

    UFUNCTION()
    void HandleRetryClicked();

    UFUNCTION()
    void HandleSettingsClicked();

    UFUNCTION()
    void HandleSaveAndQuitClicked();
};
