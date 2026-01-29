#pragma once

#include "CoreMinimal.h"
#include "MountainGenSettings.h"

// d < IsoLevel : air
// d >= IsoLevel: solid
struct FVoxelDensityGenerator
{
    FVoxelDensityGenerator() = default;

    FVoxelDensityGenerator(const FMountainGenSettings& InS, const FVector& InTerrainOriginWorld)
        : S(InS)
        , TerrainOriginWorld(InTerrainOriginWorld)
        , Seed((InS.Seed <= 0) ? 1557 : InS.Seed)
    {
    }

    float SampleDensity(const FVector& WorldPosCm) const;

private:
    // ---------- helpers ----------
    static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }

    static FORCEINLINE float SmoothStep(float a, float b, float x)
    {
        const float t = FMath::Clamp((x - a) / FMath::Max(0.0001f, (b - a)), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    static FORCEINLINE float Noise3D(const FVector& p)
    {
        return FMath::PerlinNoise3D(p); // -1..1
    }

    // Seed-based deterministic offsets (world cm -> noise domain shift)
    FORCEINLINE FVector SeedOffset() const
    {
        const float a = (float)(Seed % 100000) * 0.01557f;
        const float b = (float)((Seed * 31) % 100000) * 0.01111f;
        const float c = (float)((Seed * 131) % 100000) * 0.00913f;
        return FVector(a, b, c);
    }

    // FBM [-?, +?]
    float FBM3D(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const;

    // Ridged [0,1]
    float RidgedFBM01(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const;

    // Domain warp (3D)
    FVector Warp3D(const FVector& p, float PatchCm, float AmpCm) const;

private:
    FMountainGenSettings S;
    FVector TerrainOriginWorld = FVector::ZeroVector;
    int32 Seed = 1557;
};