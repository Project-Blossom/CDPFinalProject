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
        InitSeedParams();
    }

    float SampleDensity(const FVector& WorldPosCm) const;

    float ComputeFrontInfluenceCm() const;

private:
    static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }

    static FORCEINLINE float Remap01(float x, float a, float b)
    {
        const float den = (b - a);
        if (FMath::IsNearlyZero(den)) return (x >= b) ? 1.f : 0.f;
        return Clamp01((x - a) / den);
    }

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

    void InitSeedParams()
    {
        FRandomStream Rng(Seed ^ 0x6C8E9CF5);

        const float OffsetRangeCm = 800000.0f;
        SeedOffsetCm = FVector(
            Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm),
            Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm),
            Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm)
        );

        SeedScaleMul = Rng.FRandRange(0.85f, 1.15f);
        AxisShuffle = Rng.RandRange(0, 5);
    }

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

    float FBM3D(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const
    {
        Octaves = FMath::Clamp(Octaves, 1, 12);

        float sum = 0.f;
        float amp = 1.f;
        float freq = 1.f;

        for (int32 i = 0; i < Octaves; ++i)
        {
            sum += Noise3D(p * freq) * amp;
            freq *= Lacunarity;
            amp *= Gain;
        }
        return sum;
    }

    float RidgedFBM01(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const
    {
        Octaves = FMath::Clamp(Octaves, 1, 12);

        float sum = 0.f;
        float amp = 0.5f;
        float freq = 1.f;

        for (int32 i = 0; i < Octaves; ++i)
        {
            const float n = Noise3D(p * freq);
            sum += Ridged01(n) * amp;
            freq *= Lacunarity;
            amp *= Gain;
        }
        return Clamp01(sum);
    }

    FVector Warp3D(const FVector& p, float PatchCm, float AmpCm) const
    {
        const float inv = 1.f / FMath::Max(1.f, PatchCm);
        const FVector q = p * inv;

        const float nx = FBM3D(q + FVector(13, 7, 5), 3, 2.f, 0.5f);
        const float ny = FBM3D(q + FVector(5, 11, 17), 3, 2.f, 0.5f);
        const float nz = FBM3D(q + FVector(19, 3, 29), 3, 2.f, 0.5f);

        return p + FVector(nx, ny, nz) * AmpCm;
    }

private:
    FMountainGenSettings S;
    FVector TerrainOriginWorld = FVector::ZeroVector;

    int32 Seed = 1;
    FVector SeedOffsetCm = FVector::ZeroVector;
    float SeedScaleMul = 1.0f;
    int32 AxisShuffle = 0;
};