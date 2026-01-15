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

	UPROPERTY(EditAnywhere, Category = "Glitch")
	UMaterialInterface* PostProcessMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Glitch")
	UCameraComponent* TargetCamera = nullptr;

	// ====== 실시간 조절 파라미터(Details에서 조절) ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Params", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GlitchIntensity = 0.0f;

	// 화면 줄 글리치(스트립) 간격: 낮출수록 간격 넓어짐(줄 개수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Params", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	float StripCount = 80.0f;

	// 스캔라인 촘촘함: 낮출수록 간격 넓어짐
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Params", meta = (ClampMin = "1.0", ClampMax = "5000.0"))
	float ScanFreq = 800.0f;

	// (선택) 이미 쓰고 있으면 같이 노출
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Params", meta = (ClampMin = "0.0", ClampMax = "0.02"))
	float RGBShift = 0.004f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Params", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float BlockShift = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Params", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScanIntensity = 0.08f;

	// ====== 머티리얼 파라미터 이름(오타 방지/교체 가능) ======
	UPROPERTY(EditAnywhere, Category = "Glitch|ParamNames")
	FName Param_GlitchIntensity = TEXT("GlitchIntensity");

	UPROPERTY(EditAnywhere, Category = "Glitch|ParamNames")
	FName Param_StripCount = TEXT("StripCount");

	UPROPERTY(EditAnywhere, Category = "Glitch|ParamNames")
	FName Param_ScanFreq = TEXT("ScanFreq");

	UPROPERTY(EditAnywhere, Category = "Glitch|ParamNames")
	FName Param_RGBShift = TEXT("RGBShift");

	UPROPERTY(EditAnywhere, Category = "Glitch|ParamNames")
	FName Param_BlockShift = TEXT("BlockShift");

	UPROPERTY(EditAnywhere, Category = "Glitch|ParamNames")
	FName Param_ScanIntensity = TEXT("ScanIntensity");

	// 스파이크(원하면 기존 유지)
	UFUNCTION(BlueprintCallable, Category = "Glitch")
	void TriggerSpike(float PeakIntensity = 1.0f, float Duration = 0.35f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MID = nullptr;

	int32 BlendableIndex = INDEX_NONE;

	float SpikeTimeLeft = 0.0f;
	float SpikeDuration = 0.0f;
	float SpikePeak = 0.0f;

	void EnsureCamera();
	void ApplyToCamera();
	void RemoveFromCamera();
	void UpdateParameters(float DeltaTime);

	// 변경이 있을 때만 SetScalarParameterValue를 줄이고 싶으면 캐시
	void PushAllParamsToMID(float FinalIntensity);
};
