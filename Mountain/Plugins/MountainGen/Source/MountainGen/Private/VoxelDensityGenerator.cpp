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
    const float den = FMath::Max(0.0001f, (b - a));
    float t = FMath::Clamp((x - a) / den, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.0f, 1.0f); }

// 3D FBM (-..+)
static FORCEINLINE float FBM3D(float x, float y, float z, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f, amp = 1.0f, freq = 1.0f;
    for (int32 i = 0; i < Octaves; ++i)
    {
        sum += Noise3D(x * freq, y * freq, z * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum;
}

static FORCEINLINE float Ridged01(float n)
{
    float r = 1.0f - FMath::Abs(n);
    r = Clamp01(r);
    return r * r;
}

static FORCEINLINE float RidgedFBM01(float x, float y, float z, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f, amp = 0.5f, freq = 1.0f;
    for (int32 i = 0; i < Octaves; ++i)
    {
        float n = Noise3D(x * freq, y * freq, z * freq); // -1..1
        sum += Ridged01(n) * amp;                        // 0..+
        freq *= Lacunarity;
        amp *= Gain;
    }
    return Clamp01(sum); // 0..1
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPos) const
{
    const FVector P = WorldPos - TerrainOrigin;
    float x = P.X;
    float y = P.Y;
    float z = P.Z;

    const float SOff = SeedOffsetCm();

    // -----------------------------------------
    // (A) Domain Warp
    // -----------------------------------------
    if (S.WarpStrength > 0.0f && S.WarpAmpCm > 0.0f && S.WarpPatchCm > 1.0f)
    {
        const float WarpFreq = 1.0f / FMath::Max(1.0f, S.WarpPatchCm);
        const float W = S.WarpAmpCm * S.WarpStrength;

        const float wx = Noise3D((x + SOff) * WarpFreq, (y + SOff) * WarpFreq, (z + 0.13f * SOff) * WarpFreq);
        const float wy = Noise3D((x - SOff) * WarpFreq, (y + SOff) * WarpFreq, (z - 0.17f * SOff) * WarpFreq);
        const float wz = Noise3D((x + SOff) * WarpFreq, (y - SOff) * WarpFreq, (z + 0.19f * SOff) * WarpFreq);

        x += wx * W;
        y += wy * W;
        z += wz * (W * 0.15f);
    }

    // -----------------------------------------
    // (B) 지면(2D) + 3D 혼합
    // -----------------------------------------
    const float WorldScale = FMath::Max(1000.0f, S.WorldScaleCm);
    const float DetailScale = FMath::Clamp(S.DetailScaleCm, 300.0f, WorldScale);

    const float Fw = 1.0f / WorldScale;
    const float Fd = 1.0f / DetailScale;

    const float h2 = Noise2D((x + SOff) * Fw, (y - SOff) * Fw); // -1..1
    const float surfaceZ = S.BaseHeightCm + h2 * S.HeightAmpCm;

    float big01 = RidgedFBM01(
        (x + SOff) * Fw,
        (y - SOff) * Fw,
        (z + SOff) * Fw,
        5, 2.0f, 0.55f
    );
    float bigSigned = big01 * 2.0f - 1.0f; // -1..1

    float mid = FBM3D(
        (x - SOff) * Fd,
        (y + SOff) * Fd,
        (z + 0.5f * SOff) * Fd,
        4, 2.15f, 0.52f
    ); // -..+

    const float MassAmp = FMath::Max(500.0f, S.BaseField3DStrengthCm);
    float massField = bigSigned * MassAmp + mid * (0.30f * MassAmp);

    // -----------------------------------------
    // (C) 중력/고도
    // -----------------------------------------
    const float Gravity = FMath::Max(0.05f, S.GravityStrength);

    float density = (z - surfaceZ);

    density -= massField * 0.0008f; 

    density += (z - S.BaseHeightCm) * (Gravity * 0.015f);

    // -----------------------------------------
    // (D) Overhang
    // -----------------------------------------
    if (S.VolumeStrength > 0.0f && S.OverhangScaleCm > 1.0f)
    {
        const float OverFreq = 1.0f / FMath::Max(1.0f, S.OverhangScaleCm);

        float n01 = RidgedFBM01(
            (x + 0.7f * SOff) * OverFreq,
            (y - 0.4f * SOff) * OverFreq,
            (z + 0.2f * SOff) * OverFreq,
            4, 2.0f, 0.55f
        );

        float mask = (n01 - S.OverhangBias) / FMath::Max(0.0001f, (1.0f - S.OverhangBias));
        mask = Clamp01(mask);
        mask = FMath::Pow(mask, 2.2f);

        density -= mask * S.VolumeStrength * (S.OverhangDepthCm * 0.02f);
    }

    // -----------------------------------------
    // (E) Cave
    // -----------------------------------------
    if (S.CaveStrength > 0.0f && S.CaveScaleCm > 1.0f)
    {
        const float hUp = SmoothStep(S.CaveMinHeightCm, S.CaveMinHeightCm + 4000.0f, z);
        const float hDown = 1.0f - SmoothStep(S.CaveMaxHeightCm, S.CaveMaxHeightCm + 4000.0f, z);
        const float HeightBand = hUp * hDown;

        if (HeightBand > 0.001f)
        {
            const float CaveFreq = 1.0f / FMath::Max(1.0f, S.CaveScaleCm);

            float c = FBM3D(
                (x + 1.3f * SOff) * CaveFreq,
                (y - 0.9f * SOff) * CaveFreq,
                (z + 0.6f * SOff) * CaveFreq,
                5, 2.0f, 0.5f
            ); // -..+

            float c01 = c * 0.5f + 0.5f; // 0..1

            float m = (c01 - S.CaveThreshold) / FMath::Max(0.0001f, S.CaveBand);
            m = Clamp01(m);
            m = FMath::Pow(m, 1.8f);

            density += m * S.CaveStrength * HeightBand * (S.CaveDepthCm * 0.02f);
        }
    }

    return density;
}