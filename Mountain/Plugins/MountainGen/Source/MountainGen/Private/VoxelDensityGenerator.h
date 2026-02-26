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
        InitCachedConstants();
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
    void InitCachedConstants();

    struct FCached
    {
        float Iso = 0.f;
        float Voxel = 1.f;

        float BaseFieldAmp = 0.f;

        float CliffH = 1000.f;
        float FrontX = 200.f;

        float OverhangBand = 0.f;
        float OverhangScale = 200.f;
        float OverhangBias = 0.f;
        float OverhangAmp = 0.f;

        float Base3DScale = 2000.f;
        int32 Base3DOct = 1;

        float DetailScale = 600.f;
        int32 DetailOct = 1;

        float RoughBand = 150.f;
        float RoughScale = 300.f;
    };

    FCached C;

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