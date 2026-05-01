#pragma once

#include "CoreMinimal.h"
#include "MountainGenSettings.h"

struct FMGMetrics
{
    float OverhangRatio = 0.f;     // 0..1
    float SteepRatio = 0.f;        // 0..1

    // 0..1.
    float RoughnessRatio = 0.f;

    // 0..1.
    float ShadowRiskRatio = 0.f;

    int32 SurfaceNearSamples = 0;
};

struct FMGClampRange
{
    float Min = 0.f;
    float Max = 0.f;

    FORCEINLINE float Clamp(float v) const
    {
        return FMath::Clamp(v, Min, Max);
    }
};

struct FMGPresetBounds
{
    FMGClampRange BaseField3DStrengthCm;
    FMGClampRange BaseField3DScaleCm;
    FMGClampRange DetailScaleCm;
    FMGClampRange DetailStrengthCm;
    FMGClampRange SurfaceRoughnessStrengthCm;
    FMGClampRange SurfaceRoughnessMaskStrength;
    FMGClampRange SurfaceQualityScoreWeight;

    FMGClampRange VolumeStrength;
    FMGClampRange OverhangScaleCm;
    FMGClampRange OverhangBias;
    FMGClampRange OverhangDepthCm;
    FMGClampRange OverhangFadeCm;
};

struct FMGDifficultyPreset
{
    FMGTargets Targets;
    FMGPresetBounds Bounds;

    float BaseField3DStrengthCm = 12000.f;
    float BaseField3DScaleCm = 16000.f;
    float DetailScaleCm = 6000.f;
    float DetailStrengthCm = 0.f;
    float SurfaceRoughnessStrengthCm = 0.f;
    float SurfaceRoughnessMaskStrength = 0.75f;
    float SurfaceQualityScoreWeight = 1.0f;

    float VolumeStrength = 1.0f;
    float OverhangScaleCm = 8000.f;
    float OverhangBias = 0.55f;
    float OverhangDepthCm = 2500.f;
    float OverhangFadeCm = 15000.f;
};

FMGDifficultyPreset MGMakeDifficultyPreset(EMountainGenDifficulty D);

void MGApplyDifficultyPreset(FMountainGenSettings& S);

void MGClampToDifficultyBounds(FMountainGenSettings& S);

//Metrics
FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax);

// AutoTune
void MGAutoTuneIntentParams(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax);

// SeedSearch
int32 MGSearchSeedForTargets(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax,
    int32 InputSeed,
    int32 Tries,
    bool bRetryUntilSatisfied,
    int32 MaxAttempts,
    bool bDebug,
    int32 DebugEveryN,
    TFunction<void(const FString&, float, FColor)> DebugPrint);

void MGDeriveReproducibleDomainFromSeed(FMountainGenSettings& InOutS, int32 Seed);