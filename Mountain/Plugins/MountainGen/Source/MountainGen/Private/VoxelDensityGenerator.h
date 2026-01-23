#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.h"

class FVoxelDensityGenerator
{
public:
    FVoxelDensityGenerator(const FMountainGenSettings& InSettings, const FVector& InTerrainOrigin)
        : S(InSettings), TerrainOrigin(InTerrainOrigin)
    {
    }

    float SampleDensity(const FVector& WorldPos) const;

private:
    const FMountainGenSettings& S;
    FVector TerrainOrigin;

    FORCEINLINE float SeedOffsetCm() const
    {
        return (float)(S.Seed % 100000) * 0.01f;
    }
};