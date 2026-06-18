#include "UI/AltitudeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SWidget.h"

void UAltitudeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayerMarker)
	{
		PlayerMarker->SetVisibility(ESlateVisibility::Visible);
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);
	}

	if (AltitudeText)
	{
		AltitudeText->SetText(FText::FromString(TEXT("0m")));
		AltitudeText->SetVisibility(ESlateVisibility::Visible);
	}

	GlitchTimer = 0.0f;
	NextGlitchTime = FMath::RandRange(0.1f, 0.3f);

	// 초기 위치: Alpha=0 (최저 고도) 기준으로 설정
	// NativeConstruct 시점에는 Slot이 준비되지 않을 수 있으므로
	// 타이머로 한 프레임 뒤에 초기화
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (IsValid(this))
			{
				UpdateMarkerPosition(0.0f);
				UE_LOG(LogTemp, Log, TEXT("AltitudeWidget: Initial position set (deferred)"));
			}
		});
	}

	UE_LOG(LogTemp, Log, TEXT("AltitudeWidget constructed"));
}

void UAltitudeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bGlitchMode)
	{
		UpdateGlitchEffect(InDeltaTime);
	}
}

void UAltitudeWidget::EnableGlitchMode()
{
	if (bGlitchMode) return;

	bGlitchMode = true;
	GlitchTimer = 0.0f;
	NextGlitchTime = FMath::RandRange(0.05f, 0.2f);
	FlickerTimer = 0.0f;
	bIsVisible = true;

	UE_LOG(LogTemp, Warning, TEXT("AltitudeWidget: Glitch Mode ENABLED"));
}

void UAltitudeWidget::DisableGlitchMode()
{
	if (!bGlitchMode) return;

	bGlitchMode = false;

	if (PlayerMarker)
	{
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
		{
			PanelSlot->SetPosition(NormalMarkerPos);
		}
	}

	if (AltitudeText)
	{
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(AltitudeText->Slot))
		{
			PanelSlot->SetPosition(NormalMarkerPos + TextOffset);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("AltitudeWidget: Glitch Mode DISABLED"));
}

void UAltitudeWidget::UpdateAltitude(float CurrentZ, float GroundZ, float MaxHeight)
{
	float RelativeHeight = FMath::Max(0.0f, CurrentZ - GroundZ);

	float Percentage = 0.0f;
	if (MaxHeight > 0.0f)
	{
		Percentage = FMath::Clamp(RelativeHeight / MaxHeight, 0.0f, 1.0f);
	}

	float Meters = RelativeHeight / 100.0f;

	if (!bGlitchMode)
	{
		UpdateMarkerPosition(Percentage);
		UpdateAltitudeText(Meters);
	}
}

FVector2D UAltitudeWidget::CalcArcPosition(float Alpha) const
{
	// Alpha 0 = 최저(ArcEndAngle), 1 = 최고(ArcStartAngle)
	float Theta = FMath::Lerp(ArcEndAngle, ArcStartAngle, Alpha);
	float ThetaRad = FMath::DegreesToRadians(Theta);

	FVector2D Pos;
	Pos.X = ArcCenter.X + ArcRadius * FMath::Cos(ThetaRad);
	Pos.Y = ArcCenter.Y + ArcRadius * FMath::Sin(ThetaRad);
	return Pos;
}

void UAltitudeWidget::UpdateMarkerPosition(float Percentage)
{
	if (!PlayerMarker) return;

	FVector2D MarkerPos = CalcArcPosition(Percentage);
	NormalMarkerPos = MarkerPos;

	UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot);
	if (PanelSlot)
	{
		PanelSlot->SetPosition(MarkerPos);
		UE_LOG(LogTemp, Warning, TEXT("AltitudeWidget: Marker→(%.1f, %.1f) ArcCenter=(%.1f,%.1f) R=%.1f Alpha=%.2f"),
			MarkerPos.X, MarkerPos.Y, ArcCenter.X, ArcCenter.Y, ArcRadius, Percentage);
	}
	else
	{
		// Cast 실패 — PlayerMarker가 Canvas Panel 직접 자식이 아님
		UE_LOG(LogTemp, Error, TEXT("AltitudeWidget: PlayerMarker is NOT a direct child of Canvas Panel! Slot type: %s"),
			PlayerMarker->Slot ? *PlayerMarker->Slot->GetClass()->GetName() : TEXT("NULL"));
	}

	if (AltitudeText)
	{
		UCanvasPanelSlot* TextPanelSlot = Cast<UCanvasPanelSlot>(AltitudeText->Slot);
		if (TextPanelSlot)
		{
			TextPanelSlot->SetPosition(MarkerPos + TextOffset);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AltitudeWidget: AltitudeText is NOT a direct child of Canvas Panel! Slot type: %s"),
				AltitudeText->Slot ? *AltitudeText->Slot->GetClass()->GetName() : TEXT("NULL"));
		}
	}
}

void UAltitudeWidget::UpdateAltitudeText(float Meters)
{
	if (!AltitudeText) return;

	FString AltitudeString = FString::Printf(TEXT("%.0fm"), Meters);
	AltitudeText->SetText(FText::FromString(AltitudeString));
}

void UAltitudeWidget::UpdateGlitchEffect(float DeltaTime)
{
	GlitchTimer += DeltaTime;

	if (GlitchTimer >= NextGlitchTime)
	{
		GlitchMarkerPosition();
		GlitchMarkerColor();
		GlitchAltitudeText();

		GlitchTimer = 0.0f;
		NextGlitchTime = FMath::RandRange(0.05f, 0.2f);
	}

	FlickerTimer += DeltaTime;
	if (FlickerTimer >= 0.05f)
	{
		bIsVisible = !bIsVisible;
		FlickerTimer = 0.0f;

		if (PlayerMarker)
		{
			FLinearColor CurrentColor = PlayerMarker->GetColorAndOpacity();
			CurrentColor.A = bIsVisible ? 1.0f : 0.0f;
			PlayerMarker->SetColorAndOpacity(CurrentColor);
		}
	}
}

void UAltitudeWidget::GlitchMarkerPosition()
{
	if (!PlayerMarker) return;

	float RandomAlpha = FMath::RandRange(0.0f, 1.0f);
	FVector2D GlitchPos = CalcArcPosition(RandomAlpha);

	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
	{
		PanelSlot->SetPosition(GlitchPos);
	}

	if (AltitudeText)
	{
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(AltitudeText->Slot))
		{
			PanelSlot->SetPosition(GlitchPos + TextOffset);
		}
	}
}

void UAltitudeWidget::GlitchMarkerColor()
{
	if (!PlayerMarker) return;

	FLinearColor RandomColor;
	RandomColor.R = FMath::RandRange(0.0f, 1.0f);
	RandomColor.G = FMath::RandRange(0.0f, 1.0f);
	RandomColor.B = FMath::RandRange(0.0f, 1.0f);
	RandomColor.A = 1.0f;
	PlayerMarker->SetColorAndOpacity(RandomColor);
}

void UAltitudeWidget::GlitchAltitudeText()
{
	if (!AltitudeText) return;

	int32 RandomMeters = FMath::RandRange(-8000, 16000);
	AltitudeText->SetText(FText::FromString(FString::Printf(TEXT("%dm"), RandomMeters)));
}

int32 UAltitudeWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 부모 클래스 먼저 그리기
	int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// 곡선 경로 점 목록 생성
	// ArcSegments 개수로 ArcEndAngle ~ ArcStartAngle 사이를 균등 분할
	TArray<FVector2D> Points;
	Points.Reserve(ArcSegments + 1);

	for (int32 i = 0; i <= ArcSegments; i++)
	{
		float Alpha = (float)i / (float)ArcSegments;
		// Alpha 0 = ArcEndAngle (하단), 1 = ArcStartAngle (상단)
		float Theta = FMath::Lerp(ArcEndAngle, ArcStartAngle, Alpha);
		float ThetaRad = FMath::DegreesToRadians(Theta);

		FVector2D Pos;
		Pos.X = ArcCenter.X + ArcRadius * FMath::Cos(ThetaRad);
		Pos.Y = ArcCenter.Y + ArcRadius * FMath::Sin(ThetaRad);
		Points.Add(Pos);
	}

	// FSlateDrawElement::MakeLines로 곡선 그리기
	// AllottedGeometry의 로컬 좌표 기준이므로 별도 변환 불필요
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

	return MaxLayer + 1;
}
