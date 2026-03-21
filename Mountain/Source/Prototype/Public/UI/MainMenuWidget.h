#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 메인 메뉴 UI
 */
UCLASS()
class PROTOTYPE_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 버튼 클릭 이벤트
    UFUNCTION(BlueprintCallable, Category = "Main Menu")
    void OnNewGameClicked();

    UFUNCTION(BlueprintCallable, Category = "Main Menu")
    void OnLoadGameClicked();

    UFUNCTION(BlueprintCallable, Category = "Main Menu")
    void OnSettingsClicked();

    UFUNCTION(BlueprintCallable, Category = "Main Menu")
    void OnQuitClicked();

protected:
    virtual void NativeConstruct() override;

    // Blueprint에서 바인딩할 버튼들
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* NewGameButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* LoadGameButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* SettingsButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* QuitButton;

    // 세이브 슬롯 선택 레벨
    UPROPERTY(EditDefaultsOnly, Category = "Main Menu")
    FName SaveSlotSelectionLevel = FName("SaveSlotSelection");

    // Settings 레벨
    UPROPERTY(EditDefaultsOnly, Category = "Main Menu")
    FName SettingsLevel = FName("Settings");

private:
    UFUNCTION()
    void HandleNewGameClicked();

    UFUNCTION()
    void HandleLoadGameClicked();

    UFUNCTION()
    void HandleSettingsClicked();

    UFUNCTION()
    void HandleQuitClicked();
};
