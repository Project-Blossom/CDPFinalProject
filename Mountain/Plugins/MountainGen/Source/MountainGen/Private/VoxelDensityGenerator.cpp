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

static FORCEINLINE float Clamp01(float x)
{
    return FMath::Clamp(x, 0.0f, 1.0f);
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
    const float OverFreq = 1.0f / FMath::Max(1.0f, S.OverhangScaleCm);

    const float WarpFreq = 1.0f / FMath::Max(1.0f, S.WarpPatchCm);

    const float Base3DFreq = 1.0f / FMath::Max(100.0f, S.BaseField3DScaleCm);

    // ------------------------------------------------------------
    // (1) Domain Warp
    // ------------------------------------------------------------
    float wx2 = wx, wy2 = wy, wz2 = wz;
    if (S.WarpStrength > 0.0f && S.WarpAmpCm > 0.0f)
    {
        const float W = S.WarpAmpCm * S.WarpStrength;

        const float wnx = Noise3D((wx + SOff) * WarpFreq, (wy + SOff) * WarpFreq, 0.0f);
        const float wny = Noise3D((wx - SOff) * WarpFreq, (wy + SOff) * WarpFreq, 0.0f);
        const float wnz = Noise3D((wx + SOff) * WarpFreq, (wy - SOff) * WarpFreq, 0.0f);

        wx2 = wx + wnx * W;
        wy2 = wy + wny * W;
        wz2 = wz + wnz * (W * 0.25f);
    }

    // ------------------------------------------------------------
    // (2) 2D 기반 높이
    // ------------------------------------------------------------
    const float h0 = FBM2D((wx2 + SOff) * WorldFreq, (wy2 + SOff) * WorldFreq, 5, 2.0f, 0.5f);
    const float h1 = FBM2D((wx2 - SOff) * DetailFreq, (wy2 + SOff) * DetailFreq, 4, 2.2f, 0.55f);

    const float RampT = FMath::Clamp(wx2 / FMath::Max(1.0f, S.RampLengthCm), 0.0f, 1.0f);
    const float Ramp = RampT * S.RampHeightCm;

    const float SurfaceHeight =
        S.BaseHeightCm
        + Ramp
        + (h0 * S.HeightAmpCm * 0.55f)
        + (h1 * S.HeightAmpCm * S.SteepnessDetailFactor);

    float density = (wz2 - SurfaceHeight);

    // ------------------------------------------------------------
    // (2.5) Base 3D Field
    // ------------------------------------------------------------
    if (S.BaseField3DStrengthCm > 0.0f)
    {
        const int32 Oct = FMath::Clamp(S.BaseField3DOctaves, 1, 8);

        const float n3 = FBM3D(
            (wx2 + SOff) * Base3DFreq,
            (wy2 - SOff) * Base3DFreq,
            (wz2 + SOff) * Base3DFreq,
            Oct, 2.0f, 0.5f
        );

        float r = Ridged(n3);
        r = Clamp01(r);
        r = FMath::Pow(r, FMath::Clamp(S.BaseField3DRidgedPower, 1.0f, 4.0f));

        const float disp01 = (r * 2.0f - 1.0f); // -1..1
        density += disp01 * S.BaseField3DStrengthCm;
    }

    // ------------------------------------------------------------
    // (3) Overhang
    // ------------------------------------------------------------
    const float Above = (wz2 - SurfaceHeight);
    const float NearSurface = 1.0f - SmoothStep(0.0f, FMath::Max(1.0f, S.OverhangFadeCm), FMath::Abs(Above));

    if (S.VolumeStrength > 0.0f && NearSurface > 0.001f)
    {
        const float n = FBM3D(
            (wx2 + SOff) * OverFreq,
            (wy2 + SOff) * OverFreq,
            (wz2 + SOff) * OverFreq,
            5, 2.0f, 0.5f
        );

        float r = Ridged(n);
        r = Clamp01(r);

        float ridgeMask = (r - S.OverhangBias) / FMath::Max(0.0001f, (1.0f - S.OverhangBias));
        ridgeMask = Clamp01(ridgeMask);
        ridgeMask = FMath::Pow(ridgeMask, 2.2f);

        const float carve = ridgeMask * NearSurface * S.VolumeStrength;
        density += carve * S.OverhangDepthCm;
    }

    // ------------------------------------------------------------
    // (4) Cave
    // ------------------------------------------------------------
    if (S.CaveStrength > 0.0f)
    {
        const float hMaskUp = SmoothStep(S.CaveMinHeightCm, S.CaveMinHeightCm + 4000.0f, wz2);
        const float hMaskDown = 1.0f - SmoothStep(S.CaveMaxHeightCm, S.CaveMaxHeightCm + 4000.0f, wz2);
        const float HeightBand = hMaskUp * hMaskDown;

        const float CaveNearSurface =
            (S.CaveNearSurfaceCm <= 0.0f)
            ? 1.0f
            : (1.0f - SmoothStep(0.0f, S.CaveNearSurfaceCm, FMath::Abs(Above)));

        if (HeightBand > 0.001f && CaveNearSurface > 0.001f)
        {
            const float c = FBM3D(
                (wx2 + SOff) * CaveFreq,
                (wy2 - SOff) * CaveFreq,
                (wz2 + SOff) * CaveFreq,
                5, 2.0f, 0.5f
            );

            const float c01 = (c * 0.5f + 0.5f);

            float mask = (c01 - S.CaveThreshold) / FMath::Max(0.0001f, S.CaveBand);
            mask = Clamp01(mask);
            mask = FMath::Pow(mask, 1.8f);

            density += mask * S.CaveStrength * HeightBand * CaveNearSurface * S.CaveDepthCm;
        }
    }

    return density;
}