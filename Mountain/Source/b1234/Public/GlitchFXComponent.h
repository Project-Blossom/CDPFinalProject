#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlitchEffectComponent.generated.h"

class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class B1234_API UGlitchEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGlitchEffectComponent();

	// 글리치용 PostProcess Material Instance(예: MI_PP_Glitch) 넣기
	UPROPERTY(EditAnywhere, Category = "Glitch")
	UMaterialInterface* PostProcessMaterial = nullptr;

	// 어느 카메라에 적용할지 (비워두면 Owner에서 자동 탐색)
	UPROPERTY(EditAnywhere, Category = "Glitch")
	UCameraComponent* TargetCamera = nullptr;

	// 현재 강도(0~1). Tick에서 부드럽게 반영
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Intensity = 0.0f;

	// 스파이크: duration 동안 intensity를 잠깐 올렸다가 내림
	UFUNCTION(BlueprintCallable, Category = "Glitch")
	void TriggerSpike(float PeakIntensity = 1.0f, float Duration = 0.35f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MID = nullptr;

	int32 BlendableIndex = INDEX_NONE; // Camera PP에 넣은 인덱스(제거/업데이트용)

	// 스파이크 제어
	float SpikeTimeLeft = 0.0f;
	float SpikeDuration = 0.0f;
	float SpikePeak = 0.0f;

	void EnsureCamera();
	void ApplyToCamera();
	void RemoveFromCamera();
	void UpdateParameters(float DeltaTime);
};
