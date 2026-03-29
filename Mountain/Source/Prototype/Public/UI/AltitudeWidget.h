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

private:
	// UI 업데이트 헬퍼
	void UpdateMarkerPosition(float Percentage);
	void UpdateAltitudeText(float Meters);

	// Insanity 효과
	void UpdateGlitchEffect(float DeltaTime);
	void GlitchMarkerPosition();
	void GlitchMarkerColor();
	void GlitchAltitudeText(); 

	// 산 이미지의 세로 높이 (픽셀)
	float MountainImageHeight = 600.0f;

	// 산 이미지의 가로 폭 (픽셀)
	float MountainImageWidth = 408.0f;

	// Glitch 모드 활성화 여부
	bool bGlitchMode = false;

	// Glitch 타이머
	float GlitchTimer = 0.0f;
	float NextGlitchTime = 0.0f;

	// 점멸 타이머
	float FlickerTimer = 0.0f;
	bool bIsVisible = true;

	// 정상 위치 저장 (Glitch 모드 종료 시 복원용)
	float NormalPositionY = 0.0f;
};