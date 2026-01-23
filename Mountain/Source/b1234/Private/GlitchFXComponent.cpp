#include "GlitchFXComponent.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

UGlitchEffectComponent::UGlitchEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MID = nullptr;
	BlendableIndex = INDEX_NONE;

	NextGlitchIn = 0.0f;
	BurstLeft = 0;
	BurstTimer = 0.0f;
}

void UGlitchEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureCamera();
	ApplyToCamera();

	// 첫 자동 글리치까지 대기 시간 랜덤
	NextGlitchIn = FMath::FRandRange(AutoMinInterval, AutoMaxInterval);
}

void UGlitchEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveFromCamera();
	Super::EndPlay(EndPlayReason);
}

void UGlitchEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateParameters(DeltaTime);
}

void UGlitchEffectComponent::EnsureCamera()
{
	if (TargetCamera) return;
	if (AActor* Owner = GetOwner())
	{
		TargetCamera = Owner->FindComponentByClass<UCameraComponent>();
	}
}

void UGlitchEffectComponent::ApplyToCamera()
{
	if (!TargetCamera || !PostProcessMaterial) return;

	MID = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
	if (!MID) return;

	FWeightedBlendable WB;
	WB.Object = MID;
	WB.Weight = 1.0f;

	BlendableIndex = TargetCamera->PostProcessSettings.WeightedBlendables.Array.Add(WB);

	// 초기 파라미터 밀어넣기
	PushAllParamsToMID(FMath::Clamp(GlitchIntensity, 0.0f, 1.0f));
}

void UGlitchEffectComponent::RemoveFromCamera()
{
	if (!TargetCamera) return;

	if (BlendableIndex != INDEX_NONE)
	{
		auto& Arr = TargetCamera->PostProcessSettings.WeightedBlendables.Array;
		if (Arr.IsValidIndex(BlendableIndex))
		{
			Arr.RemoveAt(BlendableIndex);
		}
		BlendableIndex = INDEX_NONE;
	}

	MID = nullptr;
}

void UGlitchEffectComponent::TriggerSpike(float PeakIntensity, float Duration)
{
	SpikePeak = FMath::Clamp(PeakIntensity, 0.0f, 1.0f);
	SpikeDuration = FMath::Max(0.05f, Duration);
	SpikeTimeLeft = SpikeDuration;

	// 펄스 시작 시 프리셋 랜덤(매번 다른 패턴)
	RandomizePresetForPulse(SpikePeak);
}

void UGlitchEffectComponent::UpdateParameters(float DeltaTime)
{
	if (!MID) return;

	// --- Auto Glitch Scheduler ---
	if (bAutoGlitch)
	{
		if (BurstLeft > 0)
		{
			BurstTimer -= DeltaTime;
			if (BurstTimer <= 0.0f)
			{
				const float peak = FMath::FRandRange(AutoPeakMin, AutoPeakMax);
				const float dur = FMath::FRandRange(AutoDurMin, AutoDurMax);
				TriggerSpike(peak, dur);

				BurstLeft--;
				BurstTimer = BurstInterval;
			}
		}
		else
		{
			NextGlitchIn -= DeltaTime;
			if (NextGlitchIn <= 0.0f)
			{
				if (FMath::FRand() < BurstChance)
				{
					BurstLeft = FMath::RandRange(BurstMinCount, BurstMaxCount);
					BurstTimer = 0.0f; // 즉시 1발
				}
				else
				{
					const float peak = FMath::FRandRange(AutoPeakMin, AutoPeakMax);
					const float dur = FMath::FRandRange(AutoDurMin, AutoDurMax);
					TriggerSpike(peak, dur);
				}

				NextGlitchIn = FMath::FRandRange(AutoMinInterval, AutoMaxInterval);
			}
		}
	}

	// --- 기존 로직 ---
	float FinalIntensity = FMath::Clamp(GlitchIntensity, 0.0f, 1.0f);

	// 스파이크 (기본: 삼각파)
	if (SpikeTimeLeft > 0.0f)
	{
		SpikeTimeLeft -= DeltaTime;
		const float t = 1.0f - (SpikeTimeLeft / SpikeDuration); // 0->1
		const float tri = (t <= 0.5f) ? (t / 0.5f) : ((1.0f - t) / 0.5f);
		FinalIntensity = FMath::Max(FinalIntensity, tri * SpikePeak);
	}

	PushAllParamsToMID(FinalIntensity);
}

void UGlitchEffectComponent::PushAllParamsToMID(float FinalIntensity)
{
	if (!MID) return;

	MID->SetScalarParameterValue(Param_GlitchIntensity, FinalIntensity);
	MID->SetScalarParameterValue(Param_StripCount, StripCount);
	MID->SetScalarParameterValue(Param_ScanFreq, ScanFreq);
	MID->SetScalarParameterValue(Param_RGBShift, RGBShift);
	MID->SetScalarParameterValue(Param_BlockShift, BlockShift);
	MID->SetScalarParameterValue(Param_ScanIntensity, ScanIntensity);
}

void UGlitchEffectComponent::RandomizePresetForPulse(float Peak01)
{
	const float k = FMath::Clamp(Peak01, 0.0f, 1.0f);

	// StripCount: 강할수록 줄이 굵어지게(StripCount 낮아짐) 유도
	const float strip = FMath::Lerp(StripMax, StripMin, k) * FMath::FRandRange(0.8f, 1.2f);

	// ScanFreq: 강할 때 극단값도 나오게
	float scan = FMath::FRandRange(ScanFreqMin, ScanFreqMax);
	if (k > 0.7f && FMath::FRand() < 0.35f)
	{
		scan = (FMath::FRand() < 0.5f) ? ScanFreqMin : ScanFreqMax;
	}

	// 나머지는 강도에 비례해서 스케일
	const float rgb = FMath::FRandRange(RGBShiftMin, RGBShiftMax) * FMath::Lerp(0.6f, 1.0f, k);
	const float block = FMath::FRandRange(BlockShiftMin, BlockShiftMax) * FMath::Lerp(0.6f, 1.0f, k);
	const float scanI = FMath::FRandRange(ScanIntensityMin, ScanIntensityMax) * FMath::Lerp(0.4f, 1.0f, k);

	// 컴포넌트 값 갱신(Details에서 확인 가능)
	StripCount = strip;
	ScanFreq = scan;
	RGBShift = rgb;
	BlockShift = block;
	ScanIntensity = scanI;
}
