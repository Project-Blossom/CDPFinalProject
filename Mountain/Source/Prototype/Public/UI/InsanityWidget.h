#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InsanityWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class PROTOTYPE_API UInsanityWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Widget Components (BindWidget으로 Blueprint와 자동 연결)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerMarker;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InsanityText;

	// ── 곡선 경로 파라미터 (에디터에서 조정 가능) ──────────────────
	// ( 형태 곡선의 반지름 (픽셀)
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	float ArcRadius = 300.0f;

	// 상단 각도 (최고 Insanity) — 왼쪽 반원 기준, 음수 = 위
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	float ArcStartAngle = -80.0f;

	// 하단 각도 (최저 Insanity) — 양수 = 아래
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	float ArcEndAngle = 80.0f;

	// 곡선 중심점 (화면 UMG XY 기준, 좌측 밖에 배치)
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	FVector2D ArcCenter = FVector2D(-300.f, 400.f);

	// 마커 기준 텍스트 오프셋 (마커 왼쪽에 붙임 — Altitude와 좌우 대칭)
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	FVector2D TextOffset = FVector2D(60.f, 1.f);

	// 곡선 경로 선 색상
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	FLinearColor ArcLineColor = FLinearColor(0.8f, 0.2f, 0.2f, 0.6f);

	// 곡선 경로 선 두께
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator")
	float ArcLineThickness = 1.5f;

	// 곡선 분할 수 (값이 높을수록 부드럽지만 연산 증가, 20~40 권장)
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator", meta = (ClampMin = "8", ClampMax = "60"))
	int32 ArcSegments = 30;

	// ── 임계값 눈금 표시 ─────────────────────────────────────────
	// Insanity 0~100 스케일 기준 임계값 위치에 작은 눈금 표시
	// (예: 70 = 혼란 시작, 80 = 글리치 시작)
	UPROPERTY(EditAnywhere, Category = "InsanityIndicator|Thresholds")
	TArray<float> ThresholdValues = { 70.0f, 80.0f };

	UPROPERTY(EditAnywhere, Category = "InsanityIndicator|Thresholds")
	FLinearColor ThresholdTickColor = FLinearColor(1.0f, 0.8f, 0.2f, 0.9f);

	UPROPERTY(EditAnywhere, Category = "InsanityIndicator|Thresholds")
	float ThresholdTickLength = 12.0f;

	UPROPERTY(EditAnywhere, Category = "InsanityIndicator|Thresholds")
	float ThresholdTickThickness = 2.0f;

	// Insanity 업데이트 함수 (0~100 스칼라, GroundZ/MaxHeight 같은 정규화 불필요)
	UFUNCTION(BlueprintCallable, Category = "Insanity")
	void UpdateInsanityGauge(float CurrentInsanity, float MaxInsanity = 100.0f);

	// Glitch 모드 활성화/비활성화 (Insanity 80 경계에서만 호출)
	// 마커 좌표 이동은 하지 않고 색상 변화만 적용
	UFUNCTION(BlueprintCallable, Category = "Insanity")
	void EnableGlitchMode();

	UFUNCTION(BlueprintCallable, Category = "Insanity")
	void DisableGlitchMode();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	// UI 업데이트 헬퍼
	void UpdateMarkerPosition(float Percentage);
	void UpdateInsanityText(float CurrentInsanity);

	// Insanity Alpha(0~1) → 곡선 위 XY 좌표 계산 ( 형태: Cos 부호 반전
	FVector2D CalcArcPosition(float Alpha) const;

	// Glitch 효과 — 색상 점멸만 처리 (좌표 이동 없음)
	void UpdateGlitchEffect(float DeltaTime);
	void GlitchMarkerColor();

	// Glitch 모드 활성화 여부
	bool bGlitchMode = false;

	// Glitch 색상 전환 타이머
	float GlitchTimer = 0.0f;
	float NextGlitchTime = 0.0f;

	// 정상 마커 위치 저장 (Glitch 모드 종료 시 색상 복원에 사용)
	FVector2D NormalMarkerPos = FVector2D::ZeroVector;

	// 최근 적용된 Alpha (NativePaint에서 임계값 눈금 그릴 때 참조하지 않지만,
	// 추후 디버그/확장 용도로 보관)
	float LastAlpha = 0.0f;
};
