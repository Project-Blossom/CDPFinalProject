#include "LensDistortionComponent.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

ULensDistortionComponent::ULensDistortionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void ULensDistortionComponent::BeginPlay()
{
    Super::BeginPlay();
    Apply();
}

void ULensDistortionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Remove();
    Super::EndPlay(EndPlayReason);
}

UCameraComponent* ULensDistortionComponent::FindCamera() const
{
    if (!GetOwner()) return nullptr;
    return GetOwner()->FindComponentByClass<UCameraComponent>();
}

void ULensDistortionComponent::Apply()
{
    if (!PostProcessMaterial) return;

    CachedCamera = FindCamera();
    if (!CachedCamera) return;

    MID = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
    if (!MID) return;

    // 카메라 PostProcessSettings에 Blendable로 추가
    FWeightedBlendable WB;
    WB.Weight = Weight;
    WB.Object = MID;

    auto& Arr = CachedCamera->PostProcessSettings.WeightedBlendables.Array;
    BlendableIndex = Arr.Add(WB);

    UpdateParams();
}

void ULensDistortionComponent::Remove()
{
    if (!CachedCamera) return;

    auto& Arr = CachedCamera->PostProcessSettings.WeightedBlendables.Array;
    if (Arr.IsValidIndex(BlendableIndex) && Arr[BlendableIndex].Object == MID)
    {
        Arr.RemoveAt(BlendableIndex);
    }

    BlendableIndex = INDEX_NONE;
    MID = nullptr;
    CachedCamera = nullptr;
}

void ULensDistortionComponent::UpdateParams()
{
    if (!MID) return;

    MID->SetScalarParameterValue(TEXT("K1"), K1);
    MID->SetScalarParameterValue(TEXT("K2"), K2);

    if (CachedCamera)
    {
        auto& Arr = CachedCamera->PostProcessSettings.WeightedBlendables.Array;
        if (Arr.IsValidIndex(BlendableIndex) && Arr[BlendableIndex].Object == MID)
        {
            Arr[BlendableIndex].Weight = Weight;
        }
    }
}

void ULensDistortionComponent::SetStrength(float InK1, float InK2)
{
    K1 = InK1;
    K2 = InK2;
    UpdateParams();
}

void ULensDistortionComponent::SetWeight(float InWeight)
{
    Weight = FMath::Clamp(InWeight, 0.0f, 1.0f);
    UpdateParams();
}
