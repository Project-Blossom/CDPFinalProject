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
        , Seed(FMath::Max(1, InS.Seed))
    {
        InitSeedDomain();
    }

    float SampleDensity(const FVector& WorldPosCm) const;

private:
    static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }

    static FORCEINLINE float Noise3D(const FVector& p)
    {
        return FMath::PerlinNoise3D(p); // -1..1
    }

    static FORCEINLINE float Ridged01(float n)
    {
        float r = 1.f - FMath::Abs(n);
        r = FMath::Clamp(r, 0.f, 1.f);
        return r * r;
    }

    float FBM3D(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const;
    float RidgedFBM01(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const;

    void InitSeedDomain();

    FORCEINLINE FVector SeededDomain(const FVector& LocalCm) const
    {
        FVector p = (LocalCm + SeedOffsetCm) * SeedScaleMul;

        switch (AxisShuffle)
        {
        case 0: return FVector(p.X, p.Y, p.Z);
        case 1: return FVector(p.X, p.Z, p.Y);
        case 2: return FVector(p.Y, p.X, p.Z);
        case 3: return FVector(p.Y, p.Z, p.X);
        case 4: return FVector(p.Z, p.X, p.Y);
        case 5: return FVector(p.Z, p.Y, p.X);
        default: return p;
        }
    }

private:
    FMountainGenSettings S;
    FVector TerrainOriginWorld = FVector::ZeroVector;

    int32 Seed = 1;
    FVector SeedOffsetCm = FVector::ZeroVector;
    float SeedScaleMul = 1.0f;
    int32 AxisShuffle = 0;
};