#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndingSequenceWidget.generated.h"

class UImage;
class UButton;
class UWidgetAnimation;
class UTexture2D;
class UEndingResultWidget;

/**
 * Ending 레벨 만화 컷씬 시퀀스 (기획서 v2).
 * 흐름: 레벨 진입(검은 화면) → 이미지 1~4 순차 표시(클릭 시 FadeOut→FadeIn, 1초씩)
 *       → 이미지4 클릭 시 FadeOut(1초) → WBP_EndingResult FadeIn(1초)
 *
 * 디자이너가 WBP에서 해야 할 작업:
 *  - SequenceImage(UImage), SequenceImageButton(UButton, SequenceImage 위에 겹쳐서
 *    클릭을 받는 투명 버튼)를 배치하고 BindWidget
 *  - 위젯 애니메이션 2개 제작: FadeOutAnim(SequenceImage RenderOpacity 1→0, 1초),
 *    FadeInAnim(SequenceImage RenderOpacity 0→1, 1초) — 이름이 정확히 일치해야 자동
 *    바인딩된다
 *  - Class Defaults에서 SequenceImages 배열에 이미지 1~4 텍스처를 순서대로 등록
 *  - Class Defaults에서 EndingResultWidgetClass에 WBP_EndingResult 등록
 */
UCLASS()
class PROTOTYPE_API UEndingSequenceWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // ── 컷씬 이미지 (단일 Image 위젯, 텍스처만 교체하며 재사용) ──
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* SequenceImage;

    // SequenceImage 클릭을 받기 위한 버튼 (SequenceImage 위에 겹쳐 배치, Background 투명)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* SequenceImageButton;

    // SequenceImage의 RenderOpacity를 1→0으로 (1초)
    UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
    UWidgetAnimation* FadeOutAnim;

    // SequenceImage의 RenderOpacity를 0→1로 (1초)
    UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
    UWidgetAnimation* FadeInAnim;

    // 순서대로 표시할 컷씬 이미지 (기획서 기준 4장)
    UPROPERTY(EditDefaultsOnly, Category = "Ending Sequence")
    TArray<TObjectPtr<UTexture2D>> SequenceImages;

    // 마지막 이미지 클릭 후 띄울 결과 화면 위젯 클래스 (WBP_EndingResult)
    UPROPERTY(EditDefaultsOnly, Category = "Ending Sequence")
    TSubclassOf<UEndingResultWidget> EndingResultWidgetClass;

private:
    int32 CurrentImageIndex = 0;
    bool bTransitioning = false;

    UFUNCTION()
    void HandleSequenceImageClicked();

    UFUNCTION()
    void HandleFadeOutFinished();

    UFUNCTION()
    void HandleFadeInFinished();

    void ShowEndingResult();
};
