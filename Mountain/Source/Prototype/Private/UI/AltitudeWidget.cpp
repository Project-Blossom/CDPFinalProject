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
	}

	if (AltitudeText)
	{
		AltitudeText->SetText(FText::FromString(TEXT("0m")));
	}

	UE_LOG(LogTemp, Log, TEXT("AltitudeWidget constructed"));
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

	// UI 업데이트
	UpdateMarkerPosition(Percentage);
	UpdateAltitudeText(Meters);
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