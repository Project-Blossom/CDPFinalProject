#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.h"

struct FMGMetrics
{
    float CaveVoidRatio = 0.f;
    float OverhangRatio = 0.f;
    float SteepRatio = 0.f;

    int32 CaveSamples = 0;
    int32 SurfaceNearSamples = 0;
};

bool MGTuneSettingsFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm);