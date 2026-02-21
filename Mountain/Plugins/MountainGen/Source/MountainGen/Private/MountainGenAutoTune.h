#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.h"

struct FMGMetrics
{
    float OverhangRatio = 0.f;   // 0..1
    float SteepRatio = 0.f;      // 0..1
    int32 SurfaceNearSamples = 0;
};

// ------------------------------
// 난이도 = 허용 공간(Bounds)
// ------------------------------
struct FMGClampRange
{
    float Min = 0.f;
    float Max = 0.f;
    FORCEINLINE float Clamp(float v) const { return FMath::Clamp(v, Min, Max); }
};

struct FMGPresetBounds
{
    // 의도 파라미터 (난이도 차이를 만드는 것들만)
    FMGClampRange BaseField3DStrengthCm;
    FMGClampRange BaseField3DScaleCm;
    FMGClampRange DetailScaleCm;

    FMGClampRange VolumeStrength;
    FMGClampRange OverhangScaleCm;
    FMGClampRange OverhangBias;
    FMGClampRange OverhangDepthCm;
    FMGClampRange OverhangFadeCm;

    FMGClampRange GravityStrength;
    FMGClampRange GravityScale;
};

struct FMGDifficultyPreset
{
    FMGTargets Targets;
    FMGPresetBounds Bounds;

    // 기본값(초기점)
    float BaseField3DStrengthCm = 12000.f;
    float BaseField3DScaleCm = 16000.f;
    float DetailScaleCm = 6000.f;

    float VolumeStrength = 1.0f;
    float OverhangScaleCm = 8000.f;
    float OverhangBias = 0.55f;
    float OverhangDepthCm = 2500.f;
    float OverhangFadeCm = 15000.f;

    float GravityStrength = 1.0f;
    float GravityScale = 2.0f;
};

FMGDifficultyPreset MGMakeDifficultyPreset(EMountainGenDifficulty D);

void MGApplyDifficultyPreset(FMountainGenSettings& S);

void MGClampToDifficultyBounds(FMountainGenSettings& S);

// 빠른 Metrics
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
    const FMountainGenSettings& BaseS,
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