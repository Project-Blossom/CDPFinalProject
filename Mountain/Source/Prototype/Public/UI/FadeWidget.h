#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FadeWidget.generated.h"

/**
 * Fade In/Out 효과 Widget
 */
UCLASS()
class PROTOTYPE_API UFadeWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Fade In 시작 (검은색 → 투명)
    UFUNCTION(BlueprintCallable, Category = "Fade")
    void StartFadeIn(float Duration = 1.0f);

    // Fade Out 시작 (투명 → 검은색)
    UFUNCTION(BlueprintCallable, Category = "Fade")
    void StartFadeOut(float Duration = 2.0f);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Fade 이미지
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UImage* FadeImage;

private:
    // Fade 상태
    enum class EFadeState
    {
        None,
        FadingIn,
        FadingOut
    };

    EFadeState CurrentFadeState = EFadeState::None;

    // Fade 진행도
    float CurrentAlpha = 1.0f; // 시작은 검은 화면
    float FadeDuration = 1.0f;
    float FadeElapsedTime = 0.0f;

    // Fade 업데이트
    void UpdateFade(float DeltaTime);
};
