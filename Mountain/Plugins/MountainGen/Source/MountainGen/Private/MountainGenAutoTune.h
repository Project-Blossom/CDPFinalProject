#pragma once

#include "CoreMinimal.h"
#include "MountainGenSettings.h"

struct FMGMetrics
{
    int32 CaveSamples = 0;
    float CaveVoidRatio = 0.f;

    int32 SurfaceNearSamples = 0;
    float OverhangRatio = 0.f;
    float SteepRatio = 0.f;
};

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);

FMGMetrics MGComputeMetricsFullGrid(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);

bool MGIsSatisfiedToTargets(const FMountainGenSettings& S, const FMGMetrics& M);

bool MGTuneSettingsFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);

void MGDeriveParamsFromSeed(FMountainGenSettings& InOutS, int32 Seed);

bool MGFinalizeSettingsFromSeed(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);