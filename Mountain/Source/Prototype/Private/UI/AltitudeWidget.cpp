#include "UI/AltitudeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UAltitudeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기화
	if (PlayerMarker)
	{
		PlayerMarker->SetVisibility(ESlateVisibility::Visible);
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);
	}

	if (AltitudeText)
	{
		AltitudeText->SetText(FText::FromString(TEXT("0m")));
	}

	// Glitch 타이머 초기화
	GlitchTimer = 0.0f;
	NextGlitchTime = FMath::RandRange(0.1f, 0.3f);

	UE_LOG(LogTemp, Log, TEXT("AltitudeWidget constructed"));
}

void UAltitudeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Glitch 모드일 때만 효과 업데이트
	if (bGlitchMode)
	{
		UpdateGlitchEffect(InDeltaTime);
	}
}

void UAltitudeWidget::EnableGlitchMode()
{
	if (bGlitchMode)
		return; // 이미 활성화됨

	bGlitchMode = true;
	
	// 타이머 초기화
	GlitchTimer = 0.0f;
	NextGlitchTime = FMath::RandRange(0.05f, 0.2f);
	FlickerTimer = 0.0f;
	bIsVisible = true;

	UE_LOG(LogTemp, Warning, TEXT("AltitudeWidget: Glitch Mode ENABLED"));
}

void UAltitudeWidget::DisableGlitchMode()
{
	if (!bGlitchMode)
	{
		return;
	}

	bGlitchMode = false;

	// 마커 색상/투명도 복원
	if (PlayerMarker)
	{
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);

		FWidgetTransform Transform = PlayerMarker->GetRenderTransform();
		Transform.Translation = NormalMarkerPos;
		PlayerMarker->SetRenderTransform(Transform);
	}

	// 텍스트 위치 복원
	if (AltitudeText)
	{
		FWidgetTransform TextTransform = AltitudeText->GetRenderTransform();
		TextTransform.Translation = NormalMarkerPos + TextOffset;
		AltitudeText->SetRenderTransform(TextTransform);
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

void UAltitudeWidget::UpdateGlitchEffect(float DeltaTime)
{
	// Glitch 타이머 업데이트
	GlitchTimer += DeltaTime;

	if (GlitchTimer >= NextGlitchTime)
	{
		// 위치 랜덤 변경
		GlitchMarkerPosition();

		// 색상 랜덤 변경
		GlitchMarkerColor();

		// 고도 텍스트 랜덤 변경
		GlitchAltitudeText();

		// 다음 Glitch 시간 설정 (0.05~0.2초)
		GlitchTimer = 0.0f;
		NextGlitchTime = FMath::RandRange(0.05f, 0.2f);
	}

	// 점멸 효과 (빠른 깜빡임)
	FlickerTimer += DeltaTime;
	if (FlickerTimer >= 0.05f) // 20Hz 점멸
	{
		bIsVisible = !bIsVisible;
		FlickerTimer = 0.0f;

		if (PlayerMarker)
		{
			float Alpha = bIsVisible ? 1.0f : 0.0f;
			FLinearColor CurrentColor = PlayerMarker->GetColorAndOpacity();
			CurrentColor.A = Alpha;
			PlayerMarker->SetColorAndOpacity(CurrentColor);
		}
	}
}

void UAltitudeWidget::GlitchMarkerPosition()
{
	if (!PlayerMarker)
	{
		return;
	}

	// 곡선 경로 위에서만 랜덤 위치 선택 (Alpha 0~1 랜덤)
	float RandomAlpha = FMath::RandRange(0.0f, 1.0f);
	FVector2D GlitchPos = CalcArcPosition(RandomAlpha);

	FWidgetTransform Transform = PlayerMarker->GetRenderTransform();
	Transform.Translation = GlitchPos;
	PlayerMarker->SetRenderTransform(Transform);

	// 텍스트도 함께 이동
	if (AltitudeText)
	{
		FWidgetTransform TextTransform = AltitudeText->GetRenderTransform();
		TextTransform.Translation = GlitchPos + TextOffset;
		AltitudeText->SetRenderTransform(TextTransform);
	}
}

void UAltitudeWidget::GlitchMarkerColor()
{
	if (!PlayerMarker)
		return;

	// 랜덤 색상 생성
	FLinearColor RandomColor;
	RandomColor.R = FMath::RandRange(0.0f, 1.0f);
	RandomColor.G = FMath::RandRange(0.0f, 1.0f);
	RandomColor.B = FMath::RandRange(0.0f, 1.0f);
	RandomColor.A = 1.0f; // Alpha는 점멸 효과에서 처리

	PlayerMarker->SetColorAndOpacity(RandomColor);
}

void UAltitudeWidget::GlitchAltitudeText()
{
	if (!AltitudeText)
		return;

	// -8000 ~ 16000 범위에서 랜덤 정수 생성
	int32 RandomMeters = FMath::RandRange(-8000, 16000);

	// "랜덤값m" 형식으로 표시
	FString GlitchString = FString::Printf(TEXT("%dm"), RandomMeters);
	AltitudeText->SetText(FText::FromString(GlitchString));
}

FVector2D UAltitudeWidget::CalcArcPosition(float Alpha) const
{
	// Alpha: 0 = 최저 고도(ArcEndAngle), 1 = 최고 고도(ArcStartAngle)
	// 고도가 높을수록 ArcStartAngle(상단) 쪽으로 이동
	float Theta = FMath::Lerp(ArcEndAngle, ArcStartAngle, Alpha);
	float ThetaRad = FMath::DegreesToRadians(Theta);

	// ) 형태 곡선: ArcCenter 기준 cos(θ)=X, sin(θ)=Y
	FVector2D Pos;
	Pos.X = ArcCenter.X + ArcRadius * FMath::Cos(ThetaRad);
	Pos.Y = ArcCenter.Y + ArcRadius * FMath::Sin(ThetaRad);
	return Pos;
}

void UAltitudeWidget::UpdateMarkerPosition(float Percentage)
{
	if (!PlayerMarker)
	{
		return;
	}

	FVector2D MarkerPos = CalcArcPosition(Percentage);

	// 마커 위치 적용
	FWidgetTransform Transform = PlayerMarker->GetRenderTransform();
	Transform.Translation = MarkerPos;
	PlayerMarker->SetRenderTransform(Transform);

	// 정상 위치 저장 (Glitch 복원용)
	NormalMarkerPos = MarkerPos;

	// 고도 텍스트를 마커 오른쪽에 붙임
	if (AltitudeText)
	{
		FWidgetTransform TextTransform = AltitudeText->GetRenderTransform();
		TextTransform.Translation = MarkerPos + TextOffset;
		AltitudeText->SetRenderTransform(TextTransform);
	}
}

void UAltitudeWidget::UpdateAltitudeText(float Meters)
{
	if (!AltitudeText)
		return;

	// "250m" 형식으로 표시
	FString AltitudeString = FString::Printf(TEXT("%.0fm"), Meters);
	AltitudeText->SetText(FText::FromString(AltitudeString));
}