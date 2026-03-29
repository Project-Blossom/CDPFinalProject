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
		return; // 이미 비활성화됨

	bGlitchMode = false;

	// 정상 상태로 복원
	if (PlayerMarker)
	{
		// 색상/투명도 복원
		PlayerMarker->SetColorAndOpacity(FLinearColor::White);

		// 위치는 UpdateAltitude에서 자동 복원됨
		FWidgetTransform Transform = PlayerMarker->GetRenderTransform();
		Transform.Translation.X = 0.0f; // X 중앙으로
		Transform.Translation.Y = NormalPositionY; // 저장된 정상 위치로
		PlayerMarker->SetRenderTransform(Transform);
	}

	UE_LOG(LogTemp, Warning, TEXT("AltitudeWidget: Glitch Mode DISABLED"));
}

void UAltitudeWidget::UpdateAltitude(float CurrentZ, float GroundZ, float MaxHeight)
{
	// 지면 대비 상대 높이 계산
	float RelativeHeight = CurrentZ - GroundZ;
	RelativeHeight = FMath::Max(0.0f, RelativeHeight); // 음수 방지

	// 백분율 계산 (0~1)
	float Percentage = 0.0f;
	if (MaxHeight > 0.0f)
	{
		Percentage = FMath::Clamp(RelativeHeight / MaxHeight, 0.0f, 1.0f);
	}

	// 미터 변환 (cm → m)
	float Meters = RelativeHeight / 100.0f;

	// Glitch 모드가 아닐 때만 정상 위치/텍스트 업데이트
	if (!bGlitchMode)
	{
		UpdateMarkerPosition(Percentage);
		
		// 정상 위치 저장 (Glitch 모드 종료 시 복원용)
		if (PlayerMarker)
		{
			NormalPositionY = PlayerMarker->GetRenderTransform().Translation.Y;
		}
		
		// 정상 고도 텍스트 업데이트
		UpdateAltitudeText(Meters);
	}
	// Glitch 모드일 때는 UpdateGlitchEffect()에서 랜덤 값 처리
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
		return;

	// MountainImage 범위 내에서 랜덤 위치 생성
	// Y: -MountainImageHeight/2 ~ +MountainImageHeight/2
	float RandomY = FMath::RandRange(-MountainImageHeight * 0.5f, MountainImageHeight * 0.5f);

	// X: -MountainImageWidth/2 ~ +MountainImageWidth/2
	float RandomX = FMath::RandRange(-MountainImageWidth * 0.5f, MountainImageWidth * 0.5f);

	// Render Transform 업데이트
	FWidgetTransform Transform = PlayerMarker->GetRenderTransform();
	Transform.Translation.X = RandomX;
	Transform.Translation.Y = RandomY;
	PlayerMarker->SetRenderTransform(Transform);
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

void UAltitudeWidget::UpdateMarkerPosition(float Percentage)
{
	if (!PlayerMarker)
		return;
	
	// 산 이미지 높이의 절반에서 시작 (중앙이 0)
	float HalfHeight = MountainImageHeight * 0.5f;
    
	// Percentage: 0 = 바닥 (아래), 1 = 꼭대기 (위)
	float YPosition = HalfHeight - (Percentage * MountainImageHeight);

	// Render Transform 업데이트
	FWidgetTransform Transform = PlayerMarker->GetRenderTransform();
	Transform.Translation.Y = YPosition;
	Transform.Translation.X = 0.0f; // X는 중앙 유지
	PlayerMarker->SetRenderTransform(Transform);
}

void UAltitudeWidget::UpdateAltitudeText(float Meters)
{
	if (!AltitudeText)
		return;

	// "250m" 형식으로 표시
	FString AltitudeString = FString::Printf(TEXT("%.0fm"), Meters);
	AltitudeText->SetText(FText::FromString(AltitudeString));
}