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

void FVoxelDensityGenerator::InitCachedConstants()
{
    C.Iso = S.IsoLevel;
    C.Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    C.BaseFieldAmp = FMath::Clamp(S.BaseField3DStrengthCm, 0.f, 50000.f);

    C.CliffH = FMath::Max(1000.f, S.CliffHeightCm);
    C.FrontX = FMath::Max(200.f, S.CliffThicknessCm);

    // Overhang
    C.OverhangBand = FMath::Max(C.Voxel * 3.f, FMath::Min(S.OverhangFadeCm, 6000.f));
    C.OverhangScale = FMath::Max(200.f, S.OverhangScaleCm);
    C.OverhangBias = FMath::Clamp(S.OverhangBias, 0.0f, 1.0f);
    C.OverhangAmp = FMath::Max(0.f, S.VolumeStrength) * FMath::Max(0.f, S.OverhangDepthCm);

    // Macro 3D
    C.Base3DScale = FMath::Max(2000.f, S.BaseField3DScaleCm);
    C.Base3DOct = FMath::Clamp(S.BaseField3DOctaves, 1, 8);

    // Detail
    C.DetailScale = FMath::Max(600.f, S.DetailScaleCm);
    C.DetailOct = FMath::Clamp(S.DetailOctaves, 1, 4);

    // Roughness near surface
    C.RoughBand = FMath::Max(150.f, C.Voxel * 3.f);
    C.RoughScale = FMath::Max(300.f, S.DetailScaleCm * 0.35f);
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPosCm) const
{
    const float Iso = C.Iso;
    const FVector Local = WorldPosCm - TerrainOriginWorld;
    const FVector Domain = SeededDomain(Local);
    const float Voxel = C.Voxel;

    const float BaseFieldAmp = C.BaseFieldAmp;

    // -----------------------------
    // Cliff 기반 면
    // -----------------------------
    const float CliffH = C.CliffH;
    const float FrontX = C.FrontX;

    const float z01 = Clamp01((Local.Z - S.BaseHeightCm) / CliffH);

    float density = (FrontX - Local.X);

    // -------------------------------------------------
    // 3) Overhang / Undercut field
    // -------------------------------------------------
    {
        const float insideDepth = (FrontX - Local.X);

        const float band = C.OverhangBand;

        const float d = FMath::Max(0.f, insideDepth);
        const float nearFaceMask = 1.f - SmoothStep(0.f, band, d);

        const float mid = 1.f - FMath::Abs(2.f * z01 - 1.f);
        const float heightMask = FMath::Pow(FMath::Clamp(mid, 0.f, 1.f), 1.6f);

        const float Amp = C.OverhangAmp;

        if (Amp != 0.f && nearFaceMask != 0.f && heightMask != 0.f)
        {
            const float Scale = C.OverhangScale;
            const float r = RidgedFBM01(Domain / Scale, 5, 2.0f, 0.55f); // 0..1

            const float bias = C.OverhangBias;
            const float shaped = (r - bias);

            density += shaped * Amp * nearFaceMask * heightMask;
        }
    }

    // -----------------------------
    // Macro 3D
    // -----------------------------
    {
        if (BaseFieldAmp != 0.f)
        {
            const float Scale = C.Base3DScale;
            const int32 Oct = C.Base3DOct;

            const float n = FBM3D(Domain / Scale, Oct, 2.0f, 0.5f);
            density += n * BaseFieldAmp;
        }
    }

    // -----------------------------
    // Detail
    // -----------------------------
    {
        if (BaseFieldAmp != 0.f)
        {
            const float Scale = C.DetailScale;
            const int32 Oct = C.DetailOct;

            const float n = FBM3D(Domain / Scale, Oct, 2.0f, 0.55f);
            density += n * (0.18f * BaseFieldAmp);
        }
    }

    // -----------------------------
    //  추가 거칠기
    // -----------------------------
    {
        const float Band = C.RoughBand;
        const float surfMask = 1.f - SmoothStep(0.f, Band, FMath::Abs(density - Iso));

        if (surfMask > 0.f && BaseFieldAmp != 0.f)
        {
            const float Scale = C.RoughScale;
            const float n = FBM3D(Domain / Scale, 3, 2.0f, 0.55f);
            density += n * (0.10f * BaseFieldAmp) * surfMask;
        }
    }

    return density;
}