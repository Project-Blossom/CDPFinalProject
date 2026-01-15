#include "VoxelDensityGenerator.h"

static FORCEINLINE float Noise3D(float x, float y, float z)
{
    return FMath::PerlinNoise3D(FVector(x, y, z)); // -1..1
}

static FORCEINLINE float Noise2D(float x, float y)
{
    return FMath::PerlinNoise2D(FVector2D(x, y)); // -1..1
}

static FORCEINLINE float SmoothStep(float a, float b, float x)
{
    float t = FMath::Clamp((x - a) / FMath::Max(0.0001f, (b - a)), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static FORCEINLINE float FBM2D(float x, float y, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f, amp = 1.0f, freq = 1.0f;
    for (int i = 0; i < Octaves; ++i)
    {
        sum += Noise2D(x * freq, y * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum;
}

static FORCEINLINE float FBM3D(float x, float y, float z, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f, amp = 1.0f, freq = 1.0f;
    for (int i = 0; i < Octaves; ++i)
    {
        sum += Noise3D(x * freq, y * freq, z * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum;
}

static FORCEINLINE float Ridged(float n)
{
    return 1.0f - FMath::Abs(n);
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPos) const
{
    const FVector P = WorldPos - TerrainOrigin;

    const float wx = P.X;
    const float wy = P.Y;
    const float wz = P.Z;

    const float SOff = SeedOffsetCm();

    const float WorldFreq = 1.0f / FMath::Max(1.0f, S.WorldScaleCm);
    const float DetailFreq = 1.0f / FMath::Max(1.0f, S.DetailScaleCm);
    const float CaveFreq = 1.0f / FMath::Max(1.0f, S.CaveScaleCm);

    const float GravityStrength = S.GravityStrength;

    const float WarpStrength = S.WarpStrength;
    const float WarpAmpCm = S.WarpAmpCm;
    const float WarpPatchCm = S.WarpPatchCm;
    const float WarpFreq = 1.0f / FMath::Max(1.0f, WarpPatchCm);

    const float OverhangBias = S.OverhangBias;
    const float OverhangDepthCm = S.OverhangDepthCm;

    const float CaveMinHeightCm = S.CaveMinHeightCm;
    const float CaveMaxHeightCm = S.CaveMaxHeightCm;
    const float CaveThreshold = S.CaveThreshold;
    const float CaveBand = S.CaveBand;

    // ------------------------------------------------------------
    // (1) 2D ±â¹Ý ³ôÀÌ ÁöÇü
    // ------------------------------------------------------------
    const float h0 = FBM2D((wx + SOff) * WorldFreq, (wy + SOff) * WorldFreq, 5, 2.0f, 0.5f);
    const float h1 = FBM2D((wx - SOff) * DetailFreq, (wy + SOff) * DetailFreq, 4, 2.2f, 0.55f);

    const float RampT = FMath::Clamp(wx / FMath::Max(1.0f, S.RampLengthCm), 0.0f, 1.0f);
    const float Ramp = RampT * S.RampHeightCm;

    const float SurfaceHeight =
        S.BaseHeightCm
        + Ramp
        + (h0 * S.HeightAmpCm * 0.55f)
        + (h1 * S.HeightAmpCm * 0.18f);

    float density = (wz - SurfaceHeight);

    density -= (1.0f - SmoothStep(0.0f, 5000.0f, (wz - SurfaceHeight))) * (GravityStrength * 0.15f);

    // ------------------------------------------------------------
    // (2) Domain Warp
    // ------------------------------------------------------------
    float wx2 = wx, wy2 = wy, wz2 = wz;
    if (WarpStrength > 0.0f && WarpAmpCm > 0.0f)
    {
        const float W = WarpAmpCm * WarpStrength;

        const float wnx = Noise3D((wx + SOff) * WarpFreq, (wy + SOff) * WarpFreq, 0.0f);
        const float wny = Noise3D((wx - SOff) * WarpFreq, (wy + SOff) * WarpFreq, 0.0f);
        const float wnz = Noise3D((wx + SOff) * WarpFreq, (wy - SOff) * WarpFreq, 0.0f);

        wx2 = wx + wnx * W;
        wy2 = wy + wny * W;
        wz2 = wz + wnz * (W * 0.25f);
    }

    // ------------------------------------------------------------
    // (3) Overhang
    // ------------------------------------------------------------
    const float Above = (wz - SurfaceHeight);
    const float NearSurface = 1.0f - SmoothStep(0.0f, FMath::Max(1.0f, S.OverhangFadeCm), FMath::Abs(Above));

    if (S.VolumeStrength > 0.0f && NearSurface > 0.001f)
    {
        const float n = FBM3D((wx2 + SOff) * WorldFreq, (wy2 + SOff) * WorldFreq, (wz2 + SOff) * WorldFreq,
            3, 2.0f, 0.5f);
        const float r = Ridged(n);

        float ridgeMask = (r - OverhangBias) / FMath::Max(0.0001f, (1.0f - OverhangBias));
        ridgeMask = FMath::Clamp(ridgeMask, 0.0f, 1.0f);
        ridgeMask = ridgeMask * ridgeMask * (3.0f - 2.0f * ridgeMask);

        const float carve = ridgeMask * NearSurface * S.VolumeStrength;

        density += carve * (OverhangDepthCm / FMath::Max(1.0f, S.VoxelSizeCm));
    }

    // ------------------------------------------------------------
    // (4) ¾èÀº µ¿±¼(±¼ÅÎ)
    // ------------------------------------------------------------
    if (S.CaveStrength > 0.0f)
    {
        const float hMaskUp = SmoothStep(CaveMinHeightCm, CaveMinHeightCm + 4000.0f, wz);
        const float hMaskDown = 1.0f - SmoothStep(CaveMaxHeightCm, CaveMaxHeightCm + 4000.0f, wz);
        const float HeightBand = hMaskUp * hMaskDown;

        const float CaveNearSurface = 1.0f - SmoothStep(0.0f, 2500.0f, FMath::Abs(Above));

        if (HeightBand > 0.001f && CaveNearSurface > 0.001f)
        {
            const float c = Noise3D((wx2 + SOff) * CaveFreq, (wy2 - SOff) * CaveFreq, (wz2 + SOff) * CaveFreq);
            const float c01 = (c * 0.5f + 0.5f);

            float mask = (c01 - CaveThreshold) / FMath::Max(0.0001f, CaveBand);
            mask = FMath::Clamp(mask, 0.0f, 1.0f);
            mask = mask * mask * (3.0f - 2.0f * mask);

            density += mask * S.CaveStrength * HeightBand * CaveNearSurface * 2.0f;
        }
    }

    return density;
}