#include "UI/EndingSequenceWidget.h"
#include "UI/EndingResultWidget.h"
#include "UI/UIButtonClickSoundHelper.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"

void UEndingSequenceWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 공통 버튼 클릭음 적용 (SequenceImageButton에도 클릭음이 같이 붙는다 —
    // 컷씬 전환에 클릭음이 거슬리면 ApplyProjectButtonClickSound의 ExcludedButton
    // 인자로 SequenceImageButton을 넘기도록 바꿀 수 있다)
    PrototypeUI::ApplyProjectButtonClickSound(this);

    if (SequenceImageButton)
    {
        SequenceImageButton->OnClicked.AddDynamic(this, &UEndingSequenceWidget::HandleSequenceImageClicked);
    }

    // 위젯 애니메이션 종료 이벤트 바인딩 — FadeOut 끝나면 다음 이미지로 교체 후 FadeIn,
    // FadeIn 끝나면 다시 클릭을 받을 수 있는 상태로 전환한다.
    if (FadeOutAnim)
    {
        FWidgetAnimationDynamicEvent FadeOutEvent;
        FadeOutEvent.BindDynamic(this, &UEndingSequenceWidget::HandleFadeOutFinished);
        BindToAnimationFinished(FadeOutAnim, FadeOutEvent);
    }

    if (FadeInAnim)
    {
        FWidgetAnimationDynamicEvent FadeInEvent;
        FadeInEvent.BindDynamic(this, &UEndingSequenceWidget::HandleFadeInFinished);
        BindToAnimationFinished(FadeInAnim, FadeInEvent);
    }

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    // 첫 번째 이미지로 시작 (검은 화면 → 이미지1)
    CurrentImageIndex = 0;
    bTransitioning = false;

    if (SequenceImage && SequenceImages.IsValidIndex(0))
    {
        SequenceImage->SetBrushFromTexture(SequenceImages[0]);
    }
}

void UEndingSequenceWidget::HandleSequenceImageClicked()
{
    // 전환 애니메이션 재생 중에는 추가 클릭을 무시한다 (연타로 인한 인덱스 꼬임 방지)
    if (bTransitioning)
    {
        return;
    }

    bTransitioning = true;

    if (FadeOutAnim)
    {
        PlayAnimationForward(FadeOutAnim);
    }
    else
    {
        // 디자이너가 아직 애니메이션을 안 만들었어도 흐름 자체는 끊기지 않도록
        // 즉시 다음 단계로 넘어간다.
        HandleFadeOutFinished();
    }
}

void UEndingSequenceWidget::HandleFadeOutFinished()
{
    ++CurrentImageIndex;

    // 이미지 4(마지막) 클릭 후 FadeOut이 끝난 시점 — 결과 화면으로 전환
    if (CurrentImageIndex >= SequenceImages.Num())
    {
        ShowEndingResult();
        return;
    }

    if (SequenceImage && SequenceImages.IsValidIndex(CurrentImageIndex))
    {
        SequenceImage->SetBrushFromTexture(SequenceImages[CurrentImageIndex]);
    }

    if (FadeInAnim)
    {
        PlayAnimationForward(FadeInAnim);
    }
    else
    {
        HandleFadeInFinished();
    }
}

void UEndingSequenceWidget::HandleFadeInFinished()
{
    // 다시 클릭을 받을 수 있는 상태로
    bTransitioning = false;
}

void UEndingSequenceWidget::ShowEndingResult()
{
    if (!EndingResultWidgetClass)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("EndingSequenceWidget: EndingResultWidgetClass가 설정되지 않았습니다 (WBP Class Defaults에서 지정 필요)."));
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    UEndingResultWidget* ResultWidget = CreateWidget<UEndingResultWidget>(PC, EndingResultWidgetClass);
    if (ResultWidget)
    {
        // 컷씬 시퀀스 위젯 위에 그려지도록 ZOrder를 높게 준다.
        // ResultWidget 자신의 FadeInAnim(있다면)은 그 NativeConstruct에서 자동 재생된다.
        ResultWidget->AddToViewport(10);
    }

    RemoveFromParent();
}
