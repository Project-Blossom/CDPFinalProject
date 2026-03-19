#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageResultWidget.generated.h"

/**
 * 스테이지 클리어 결과 화면
 */
UCLASS()
class PROTOTYPE_API UStageResultWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 스테이지 정보 설정
    UFUNCTION(BlueprintCallable, Category = "Stage Result")
    void SetStageInfo(FName InStageId, float InLastTime, float InBestTime, int32 InClearCount);

    // 버튼 클릭 이벤트
    UFUNCTION(BlueprintCallable, Category = "Stage Result")
    void OnRetryClicked();

    UFUNCTION(BlueprintCallable, Category = "Stage Result")
    void OnMainMenuClicked();

    UFUNCTION(BlueprintCallable, Category = "Stage Result")
    void OnNextStageClicked();

protected:
    virtual void NativeConstruct() override;

    // Blueprint에서 바인딩할 텍스트 블록들
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* StageNameText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* LastTimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* BestTimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* ClearCountText;

    // Blueprint에서 바인딩할 버튼들
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* RetryButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* MainMenuButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* NextStageButton;

private:
    FName CurrentStageId;

    UFUNCTION()
    void HandleRetryClicked();

    UFUNCTION()
    void HandleMainMenuClicked();

    UFUNCTION()
    void HandleNextStageClicked();
};
