#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LensDistortionComponent.generated.h"

class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class B1234_API ULensDistortionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULensDistortionComponent();

    // MI_PP_LensDistortion_Default °°Àº °É ³Ö¾îµÎ¸é µÊ
    UPROPERTY(EditAnywhere, Category = "LensDistortion")
    UMaterialInterface* PostProcessMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LensDistortion", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float K1 = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LensDistortion", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float K2 = 0.00f;

    // Blendable weight (0ÀÌ¸é ²¨Áü)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LensDistortion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Weight = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "LensDistortion")
    void SetStrength(float InK1, float InK2);

    UFUNCTION(BlueprintCallable, Category = "LensDistortion")
    void SetWeight(float InWeight);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UCameraComponent* FindCamera() const;
    void Apply();
    void Remove();
    void UpdateParams();

private:
    UPROPERTY(Transient)
    UCameraComponent* CachedCamera = nullptr;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* MID = nullptr;

    int32 BlendableIndex = INDEX_NONE;
};
