#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlitchFXComponent.generated.h"

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

	// ====== Auto Glitch (불규칙 스케줄러) ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto")
	bool bAutoGlitch = true;

	// 다음 글리치까지 대기시간 범위(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.01", ClampMax = "10.0"))
	float AutoMinInterval = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.01", ClampMax = "10.0"))
	float AutoMaxInterval = 1.2f;

	// Burst(연타) 확률/횟수/간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BurstChance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "1", ClampMax = "20"))
	int32 BurstMinCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "1", ClampMax = "20"))
	int32 BurstMaxCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float BurstInterval = 0.06f; // 60ms 정도 추천

	// 펄스 강도/지속시간 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AutoPeakMin = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AutoPeakMax = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float AutoDurMin = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|Auto", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float AutoDurMax = 0.12f;

	// ====== Pulse Preset Random Range ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	float StripMin = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	float StripMax = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "1.0", ClampMax = "5000.0"))
	float ScanFreqMin = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "1.0", ClampMax = "5000.0"))
	float ScanFreqMax = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "0.0", ClampMax = "0.02"))
	float RGBShiftMin = 0.0005f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "0.0", ClampMax = "0.02"))
	float RGBShiftMax = 0.010f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float BlockShiftMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float BlockShiftMax = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScanIntensityMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch|AutoPreset", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScanIntensityMax = 1.0f;

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

	// ===== runtime state (Auto Glitch) =====
	float NextGlitchIn = 0.0f;
	int32 BurstLeft = 0;
	float BurstTimer = 0.0f;

	void EnsureCamera();
	void ApplyToCamera();
	void RemoveFromCamera();
	void UpdateParameters(float DeltaTime);

	// 변경이 있을 때만 SetScalarParameterValue를 줄이고 싶으면 캐시
	void PushAllParamsToMID(float FinalIntensity);

	// 펄스 시작 시 파라미터 프리셋 랜덤화
	void RandomizePresetForPulse(float Peak01);
};
