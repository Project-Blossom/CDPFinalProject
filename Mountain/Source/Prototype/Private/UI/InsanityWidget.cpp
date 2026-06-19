#include "UI/InsanityWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SWidget.h"

void UInsanityWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayerMarker)
	{
		PlayerMarker->SetVisibility(ESlateVisibility::Visible);
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);
	}

	if (InsanityText)
	{
		InsanityText->SetText(FText::FromString(TEXT("0")));
	}

	GlitchTimer = 0.0f;
	NextGlitchTime = FMath::RandRange(0.05f, 0.2f);

	// 초기 위치: Alpha=0 (최저 Insanity) 기준으로 설정
	// NativeConstruct 시점에는 Slot이 준비되지 않을 수 있으므로
	// 타이머로 한 프레임 뒤에 초기화
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (IsValid(this))
			{
				UpdateMarkerPosition(0.0f);
			}
		});
	}

	UE_LOG(LogTemp, Log, TEXT("InsanityWidget constructed"));
}

void UInsanityWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bGlitchMode)
	{
		UpdateGlitchEffect(InDeltaTime);
	}
}

void UInsanityWidget::EnableGlitchMode()
{
	if (bGlitchMode) return;

	bGlitchMode = true;
	GlitchTimer = 0.0f;
	NextGlitchTime = FMath::RandRange(0.05f, 0.2f);

	UE_LOG(LogTemp, Warning, TEXT("InsanityWidget: Glitch Mode ENABLED (color only, no marker jump)"));
}

void UInsanityWidget::DisableGlitchMode()
{
	if (!bGlitchMode) return;

	bGlitchMode = false;

	// 색상만 복원 (좌표는 항상 정상 위치를 유지하고 있었으므로 별도 복원 불필요)
	if (PlayerMarker)
	{
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);
	}

	UE_LOG(LogTemp, Warning, TEXT("InsanityWidget: Glitch Mode DISABLED"));
}

void UInsanityWidget::UpdateInsanityGauge(float CurrentInsanity, float MaxInsanity)
{
	float Percentage = 0.0f;
	if (MaxInsanity > 0.0f)
	{
		Percentage = FMath::Clamp(CurrentInsanity / MaxInsanity, 0.0f, 1.0f);
	}

	LastAlpha = Percentage;

	// Glitch 모드 중에도 마커 좌표는 항상 정상 위치를 따라간다.
	// (좌표 이동 효과 제거 — 색상 점멸만 유지)
	UpdateMarkerPosition(Percentage);
	UpdateInsanityText(CurrentInsanity);
}

FVector2D UInsanityWidget::CalcArcPosition(float Alpha) const
{
	// Alpha 0 = 최저(ArcEndAngle), 1 = 최고(ArcStartAngle)
	float Theta = FMath::Lerp(ArcEndAngle, ArcStartAngle, Alpha);
	float ThetaRad = FMath::DegreesToRadians(Theta);

	// ( 형태: Altitude(' )' 형태)와 달리 Cos 부호를 반전하여 왼쪽으로 볼록하게 만든다.
	FVector2D Pos;
	Pos.X = ArcCenter.X - ArcRadius * FMath::Cos(ThetaRad);
	Pos.Y = ArcCenter.Y + ArcRadius * FMath::Sin(ThetaRad);
	return Pos;
}

void UInsanityWidget::UpdateMarkerPosition(float Percentage)
{
	if (!PlayerMarker) return;

	FVector2D MarkerPos = CalcArcPosition(Percentage);
	NormalMarkerPos = MarkerPos;

	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
	{
		PanelSlot->SetPosition(MarkerPos);
	}

	if (InsanityText)
	{
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(InsanityText->Slot))
		{
			PanelSlot->SetPosition(MarkerPos + TextOffset);
		}
	}
}

void UInsanityWidget::UpdateInsanityText(float CurrentInsanity)
{
	if (!InsanityText) return;

	FString InsanityString = FString::Printf(TEXT("%.0f"), CurrentInsanity);
	InsanityText->SetText(FText::FromString(InsanityString));
}

void UInsanityWidget::UpdateGlitchEffect(float DeltaTime)
{
	// 좌표 이동(GlitchMarkerPosition)은 의도적으로 호출하지 않는다.
	// 색상 점멸 효과만 유지한다.
	GlitchTimer += DeltaTime;

	if (GlitchTimer >= NextGlitchTime)
	{
		GlitchMarkerColor();

		GlitchTimer = 0.0f;
		NextGlitchTime = FMath::RandRange(0.05f, 0.2f);
	}
}

void UInsanityWidget::GlitchMarkerColor()
{
	if (!PlayerMarker) return;

	FLinearColor RandomColor;
	RandomColor.R = FMath::RandRange(0.5f, 1.0f); // 붉은 계열 위주로 랜덤
	RandomColor.G = FMath::RandRange(0.0f, 0.4f);
	RandomColor.B = FMath::RandRange(0.0f, 0.4f);
	RandomColor.A = 1.0f;
	PlayerMarker->SetColorAndOpacity(RandomColor);
}

int32 UInsanityWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 부모 클래스 먼저 그리기
	int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// ── 곡선 경로 점 목록 생성 ──────────────────────────────────
	TArray<FVector2D> Points;
	Points.Reserve(ArcSegments + 1);

	for (int32 i = 0; i <= ArcSegments; i++)
	{
		float Alpha = (float)i / (float)ArcSegments;
		float Theta = FMath::Lerp(ArcEndAngle, ArcStartAngle, Alpha);
		float ThetaRad = FMath::DegreesToRadians(Theta);

		FVector2D Pos;
		Pos.X = ArcCenter.X - ArcRadius * FMath::Cos(ThetaRad);
		Pos.Y = ArcCenter.Y + ArcRadius * FMath::Sin(ThetaRad);
		Points.Add(Pos);
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		MaxLayer + 1,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		ArcLineColor,
		true,             // bAntialias
		ArcLineThickness
	);

	// ── 임계값 눈금 그리기 ──────────────────────────────────────
	// 각 임계값(0~100 스케일)을 Alpha로 변환해 곡선 위 접선 방향 눈금을 그린다.
	for (float ThresholdValue : ThresholdValues)
	{
		float ThresholdAlpha = FMath::Clamp(ThresholdValue / 100.0f, 0.0f, 1.0f);
		float Theta = FMath::Lerp(ArcEndAngle, ArcStartAngle, ThresholdAlpha);
		float ThetaRad = FMath::DegreesToRadians(Theta);

		FVector2D ArcPoint;
		ArcPoint.X = ArcCenter.X - ArcRadius * FMath::Cos(ThetaRad);
		ArcPoint.Y = ArcCenter.Y + ArcRadius * FMath::Sin(ThetaRad);

		// 곡선의 접선 방향(법선)을 따라 안쪽으로 짧은 눈금을 그린다.
		// 법선 방향 = 중심에서 ArcPoint를 향하는 단위 벡터
		FVector2D NormalDir = (ArcPoint - ArcCenter).GetSafeNormal();

		FVector2D TickStart = ArcPoint;
		FVector2D TickEnd = ArcPoint - NormalDir * ThresholdTickLength;

		TArray<FVector2D> TickPoints = { TickStart, TickEnd };

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			MaxLayer + 2,
			AllottedGeometry.ToPaintGeometry(),
			TickPoints,
			ESlateDrawEffect::None,
			ThresholdTickColor,
			true,
			ThresholdTickThickness
		);
	}

	return MaxLayer + 2;
}
