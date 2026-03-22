#include "UI/FadeWidget.h"
#include "Components/Image.h"

void UFadeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태: 검은 화면
    if (FadeImage)
    {
        FLinearColor Color = FLinearColor::Black;
        Color.A = CurrentAlpha;
        FadeImage->SetColorAndOpacity(Color);
    }
}

void UFadeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Fade 진행 중이면 업데이트
    if (CurrentFadeState != EFadeState::None)
    {
        UpdateFade(InDeltaTime);
    }
}

void UFadeWidget::StartFadeIn(float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("Starting Fade In (%.1f seconds)"), Duration);

    CurrentFadeState = EFadeState::FadingIn;
    FadeDuration = Duration;
    FadeElapsedTime = 0.0f;
    CurrentAlpha = 1.0f; // 검은색에서 시작
}

void UFadeWidget::StartFadeOut(float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("Starting Fade Out (%.1f seconds)"), Duration);

    CurrentFadeState = EFadeState::FadingOut;
    FadeDuration = Duration;
    FadeElapsedTime = 0.0f;
    CurrentAlpha = 0.0f; // 투명에서 시작
}

void UFadeWidget::UpdateFade(float DeltaTime)
{
    if (!FadeImage)
        return;

    FadeElapsedTime += DeltaTime;
    float Progress = FMath::Clamp(FadeElapsedTime / FadeDuration, 0.0f, 1.0f);

    if (CurrentFadeState == EFadeState::FadingIn)
    {
        // 검은색(1.0) → 투명(0.0)
        CurrentAlpha = 1.0f - Progress;
    }
    else if (CurrentFadeState == EFadeState::FadingOut)
    {
        // 투명(0.0) → 검은색(1.0)
        CurrentAlpha = Progress;
    }

    // Image Alpha 업데이트
    FLinearColor Color = FLinearColor::Black;
    Color.A = CurrentAlpha;
    FadeImage->SetColorAndOpacity(Color);

    // Fade 완료 체크
    if (Progress >= 1.0f)
    {
        CurrentFadeState = EFadeState::None;
        
        if (CurrentAlpha <= 0.01f)
        {
            // Fade In 완료 → Widget 제거
            UE_LOG(LogTemp, Warning, TEXT("Fade In Complete - Removing widget"));
            RemoveFromParent();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Fade Out Complete"));
        }
    }
}
