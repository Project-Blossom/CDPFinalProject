#include "VoxelDensityGenerator.h"

static FORCEINLINE float SmoothStep01(float t)
{
    t = FMath::Clamp(t, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

static FORCEINLINE float SmoothStep(float a, float b, float x)
{
    const float den = (b - a);
    if (FMath::IsNearlyZero(den)) return (x >= b) ? 1.f : 0.f;
    return SmoothStep01((x - a) / den);
}

float FVoxelDensityGenerator::FBM3D(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const
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

float FVoxelDensityGenerator::RidgedFBM01(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const
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

void FVoxelDensityGenerator::InitSeedDomain()
{
    FRandomStream Rng(Seed ^ 0x6C8E9CF5);

    const float OffsetRangeCm = 800000.0f;
    SeedOffsetCm = FVector(
        Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm),
        Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm),
        Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm)
    );

    const float RandomnessCap = 0.35f;
    const float Randomness = FMath::Min(Rng.FRand(), RandomnessCap);

    const float MulMin = FMath::Lerp(0.98f, 0.93f, Randomness);
    const float MulMax = FMath::Lerp(1.02f, 1.07f, Randomness);
    SeedScaleMul = Rng.FRandRange(MulMin, MulMax);

    const float PKeep = 0.85f;
    AxisShuffle = (Rng.FRand() < PKeep) ? 0 : Rng.RandRange(0, 5);
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPosCm) const
{
    const float Iso = S.IsoLevel;
    const FVector Local = WorldPosCm - TerrainOriginWorld;
    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    // -----------------------------
    // Cliff 기반 면
    // -----------------------------
    const float CliffH = FMath::Max(1000.f, S.CliffHeightCm);
    const float FrontX = FMath::Max(200.f, S.CliffThicknessCm);

    const float z01 = Clamp01((Local.Z - S.BaseHeightCm) / CliffH);

    float density = (FrontX - Local.X);

    // -----------------------------
    // Overhang
    // -----------------------------
    {
        const float curve = FMath::Pow(z01, 1.8f);

        const float OverhangAmp =
            FMath::Clamp(S.OverhangDepthCm, 0.f, 20000.f) *
            FMath::Clamp(S.VolumeStrength, 0.f, 3.0f);

        density += curve * OverhangAmp;
    }

    // -----------------------------
    // Macro 3D
    // -----------------------------
    {
        const float Scale = FMath::Max(2000.f, S.BaseField3DScaleCm);
        const int32 Oct = FMath::Clamp(S.BaseField3DOctaves, 1, 8);

        const float n = FBM3D(SeededDomain(Local) / Scale, Oct, 2.0f, 0.5f);
        const float Amp = FMath::Clamp(S.BaseField3DStrengthCm, 0.f, 50000.f);

        density += n * Amp;
    }

    // -----------------------------
    // Detail
    // -----------------------------
    {
        const float Scale = FMath::Max(600.f, S.DetailScaleCm);
        const int32 Oct = FMath::Clamp(S.DetailOctaves, 1, 4);

        const float n = FBM3D(SeededDomain(Local) / Scale, Oct, 2.0f, 0.55f);

        density += n * (0.18f * FMath::Clamp(S.BaseField3DStrengthCm, 0.f, 50000.f));
    }

    // -----------------------------
    //  추가 거칠기
    // -----------------------------
    {
        const float Band = FMath::Max(150.f, Voxel * 3.f);
        const float surfMask = 1.f - SmoothStep(0.f, Band, FMath::Abs(density - Iso));

        if (surfMask > 0.f)
        {
            const float Scale = FMath::Max(300.f, S.DetailScaleCm * 0.35f);
            const float n = FBM3D(SeededDomain(Local) / Scale, 3, 2.0f, 0.55f);
            density += n * (0.10f * FMath::Clamp(S.BaseField3DStrengthCm, 0.f, 50000.f)) * surfMask;
        }
    }

    return density;
}