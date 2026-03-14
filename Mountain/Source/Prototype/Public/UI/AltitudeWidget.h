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

protected:
	virtual void NativeConstruct() override;

private:
	// UI 업데이트 헬퍼
	void UpdateMarkerPosition(float Percentage);
	void UpdateAltitudeText(float Meters);

	// 산 이미지의 세로 높이 (픽셀)
	float MountainImageHeight = 600.0f;
};