#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AltitudeWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class PROTOTYPE_API UAltitudeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Widget Components (BindWidget으로 Blueprint와 자동 연결)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MountainImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerMarker;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AltitudeText;

	// ── 곡선 경로 파라미터 (에디터에서 조정 가능) ──────────────────
	// ) 형태 곡선의 반지름 (픽셀)
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	float ArcRadius = 300.0f;

	// 상단 각도 (최고 고도) — 오른쪽 반원 기준, 음수 = 위
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	float ArcStartAngle = -80.0f;

	// 하단 각도 (최저 고도) — 양수 = 아래
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	float ArcEndAngle = 80.0f;

	// 곡선 중심점 (화면 UMG XY 기준, 우측 밖에 배치)
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	FVector2D ArcCenter = FVector2D(1300.f, 400.f);

	// 마커 기준 텍스트 오프셋 (마커 오른쪽에 붙임)
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	FVector2D TextOffset = FVector2D(-60.f, 1.f);

	// 곡선 경로 선 색상
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	FLinearColor ArcLineColor = FLinearColor(0.8f, 0.7f, 0.5f, 0.6f);

	// 곡선 경로 선 두께
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator")
	float ArcLineThickness = 1.5f;

	// 곡선 분할 수 (값이 높을수록 부드럽지만 연산 증가, 20~40 권장)
	UPROPERTY(EditAnywhere, Category = "AltitudeIndicator", meta = (ClampMin = "8", ClampMax = "60"))
	int32 ArcSegments = 30;

	// 고도 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Altitude")
	void UpdateAltitude(float CurrentZ, float GroundZ, float MaxHeight);

	// Glitch 모드 활성화/비활성화 (Insanity 80 경계에서만 호출)
	UFUNCTION(BlueprintCallable, Category = "Altitude")
	void EnableGlitchMode();

	UFUNCTION(BlueprintCallable, Category = "Altitude")
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
	void UpdateAltitudeText(float Meters);

	// 고도 Alpha → 곡선 위 XY 좌표 계산
	FVector2D CalcArcPosition(float Alpha) const;

	// Insanity 효과
	void UpdateGlitchEffect(float DeltaTime);
	void GlitchMarkerPosition();
	void GlitchMarkerColor();
	void GlitchAltitudeText();

	// Glitch 모드 활성화 여부
	bool bGlitchMode = false;

	// Glitch 타이머
	float GlitchTimer = 0.0f;
	float NextGlitchTime = 0.0f;

	// 점멸 타이머
	float FlickerTimer = 0.0f;
	bool bIsVisible = true;

	// 정상 마커 위치 저장 (Glitch 모드 종료 시 복원용)
	FVector2D NormalMarkerPos = FVector2D::ZeroVector;
};