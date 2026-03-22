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

private:
    int32 SlotIndex = -1;

    UFUNCTION()
    void HandleConfirmClicked();

    UFUNCTION()
    void HandleCancelClicked();
};
