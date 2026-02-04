#pragma once

#include "CoreMinimal.h"
#include "MountainGenSettings.h"
#include "VoxelDensityGenerator.h"

struct FMGMetrics
{
    float CaveVoidRatio = 0.f;   // 0..1
    float OverhangRatio = 0.f;   // 0..1
    float SteepRatio = 0.f;      // 0..1

    int32 CaveSamples = 0;
    int32 SurfaceNearSamples = 0;
};

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);

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

bool MGFindFinalSeedByFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm,
    int32 StartSeed,
    int32 Tries);

bool MGIsSatisfiedToTargets(const FMountainGenSettings& S, const FMGMetrics& M);

FMGMetrics MGComputeMetricsFullGrid(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);