#include "GlitchEffectComponent.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

UGlitchEffectComponent::UGlitchEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGlitchEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureCamera();
	ApplyToCamera();
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
	PushAllParamsToMID(/*FinalIntensity=*/FMath::Clamp(GlitchIntensity, 0.0f, 1.0f));
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
}

void UGlitchEffectComponent::UpdateParameters(float DeltaTime)
{
	if (!MID) return;

	float FinalIntensity = FMath::Clamp(GlitchIntensity, 0.0f, 1.0f);

	// 스파이크 삼각파(원하면 유지)
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

	// Intensity
	MID->SetScalarParameterValue(Param_GlitchIntensity, FinalIntensity);

	// 네가 요청한 2개: ScanFreq / StripCount
	MID->SetScalarParameterValue(Param_StripCount, StripCount);
	MID->SetScalarParameterValue(Param_ScanFreq, ScanFreq);

	// 나머지(있으면 같이)
	MID->SetScalarParameterValue(Param_RGBShift, RGBShift);
	MID->SetScalarParameterValue(Param_BlockShift, BlockShift);
	MID->SetScalarParameterValue(Param_ScanIntensity, ScanIntensity);
}
MID = nullptr;
}

void UGlitchEffectComponent::TriggerSpike(float PeakIntensity, float Duration)
{
	SpikePeak = FMath::Clamp(PeakIntensity, 0.0f, 1.0f);
	SpikeDuration = FMath::Max(0.05f, Duration);
	SpikeTimeLeft = SpikeDuration;
}

void UGlitchEffectComponent::UpdateParameters(float DeltaTime)
{
	if (!MID)
	{
		return;
	}

	// 스파이크가 진행 중이면 Intensity를 임시로 덮어씀(삼각 파형)
	float FinalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	if (SpikeTimeLeft > 0.0f)
	{
		SpikeTimeLeft -= DeltaTime;
		const float t = 1.0f - (SpikeTimeLeft / SpikeDuration); // 0->1
		// 0~1~0 (삼각)
		const float tri = (t <= 0.5f) ? (t / 0.5f) : ((1.0f - t) / 0.5f);
		FinalIntensity = FMath::Max(FinalIntensity, tri * SpikePeak);
	}

	// 여기서 글리치 파라미터들을 강도에 비례해 세팅
	// 값은 취향대로 조절 가능(일단 무난한 범위)
	const float RGBShift = FinalIntensity * 0.004f;   // 0 ~ 0.004
	const float BlockShift = FinalIntensity * 0.03f;  // 0 ~ 0.03

	MID->SetScalarParameterValue(TEXT("GlitchIntensity"), FinalIntensity);
	MID->SetScalarParameterValue(TEXT("RGBShift"), RGBShift);
	MID->SetScalarParameterValue(TEXT("BlockShift"), BlockShift);
}
