#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.h"

struct FVoxelDensityGenerator
{
    const FMountainGenSettings& S;
    const FVector TerrainOrigin;

    FVoxelDensityGenerator(const FMountainGenSettings& InS, const FVector& InTerrainOrigin)
        : S(InS), TerrainOrigin(InTerrainOrigin)
    {
    }

    FORCEINLINE float SeedOffsetCm() const
    {
        return static_cast<float>(S.Seed % 100000) * 0.01557f;
    }

    float SampleDensity(const FVector& WorldPos) const;
};