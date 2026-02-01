#include "VoxelDensityGenerator.h"
#include "Math/UnrealMathUtility.h"

namespace
{
    static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }

    static FORCEINLINE float SmoothStep(float edge0, float edge1, float x)
    {
        const float den = (edge1 - edge0);
        if (FMath::IsNearlyZero(den)) return (x >= edge1) ? 1.f : 0.f;

        const float t = FMath::Clamp((x - edge0) / den, 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    }

    static FORCEINLINE float Noise3D(const FVector& p)
    {
        return FMath::PerlinNoise3D(p);
    }

    static FORCEINLINE float Ridged01(float n)
    {
        float r = 1.f - FMath::Abs(n);
        r = FMath::Clamp(r, 0.f, 1.f);
        return r * r;
    }

    static FORCEINLINE float SdBox(const FVector& p, const FVector& b)
    {
        const FVector q(FMath::Abs(p.X), FMath::Abs(p.Y), FMath::Abs(p.Z));
        const FVector d = q - b;

        const float outside = FVector(
            FMath::Max(d.X, 0.f),
            FMath::Max(d.Y, 0.f),
            FMath::Max(d.Z, 0.f)
        ).Size();

        const float inside = FMath::Min(FMath::Max(d.X, FMath::Max(d.Y, d.Z)), 0.f);
        return outside + inside;
    }

    static FORCEINLINE float Remap01(float x, float a, float b)
    {
        const float den = (b - a);
        if (FMath::IsNearlyZero(den)) return (x >= b) ? 1.f : 0.f;
        return Clamp01((x - a) / den);
    }

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
        const float n = Noise3D(p * freq); // -1..1
        sum += Ridged01(n) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return FMath::Clamp(sum, 0.f, 1.f);
}

FVector FVoxelDensityGenerator::Warp3D(const FVector& p, float PatchCm, float AmpCm) const
{
    PatchCm = FMath::Max(1.f, PatchCm);
    AmpCm = FMath::Max(0.f, AmpCm);

    const FVector so = SeedOffset();
    const float inv = 1.f / PatchCm;

    const float nx = Noise3D((p + so) * inv);
    const float ny = Noise3D((p + FVector(so.Y, so.Z, so.X)) * inv);
    const float nz = Noise3D((p + FVector(so.Z, so.X, so.Y)) * inv);

    return p + FVector(nx, ny, nz) * AmpCm;
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPosCm) const
{
    const float Iso = S.IsoLevel;

    // =========================
    // 0) Local 좌표
    // =========================
    const FVector Local = WorldPosCm - TerrainOriginWorld;

    // =========================
    // 1) 베이스
    // =========================
    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    const float HalfX = (S.ChunkX * Voxel) * 0.5f;
    const float HalfY = (S.ChunkY * Voxel) * 0.5f;
    const float Height = (S.ChunkZ * Voxel);

    const FVector BoxCenter(0.f, 0.f, (Height * 0.5f) + S.BaseHeightCm);
    const FVector P = Local - BoxCenter;

    const FVector HalfExtents(
        FMath::Max(100.f, HalfX),
        FMath::Max(100.f, HalfY),
        FMath::Max(100.f, Height * 0.5f)
    );

    float density = -SdBox(P, HalfExtents);

    if (density < -Voxel * 2.f)
        return Iso - 1.0f;

    // =========================
    // 2) 절벽
    // =========================
    const float FaceDist = (HalfExtents.X - P.X);

    const float FaceFade = FMath::Max(800.f, S.OverhangFadeCm * 0.35f);
    const float faceMask = 1.f - SmoothStep(0.f, FaceFade, FMath::Max(0.f, FaceDist));

    const float z01 = Remap01(Local.Z, S.BaseHeightCm + 500.f, S.BaseHeightCm + Height - 800.f);

    // =========================
    // 3) 도메인 워프
    // =========================
    FVector pp = Local;
    pp += SeedOffset();

    const float WarpStrength = FMath::Clamp(S.WarpStrength, 0.f, 1.2f);
    if (WarpStrength > 0.f && S.WarpAmpCm > 0.f && S.WarpPatchCm > 1.f)
    {
        const float WarpAmp = FMath::Clamp(S.WarpAmpCm, 0.f, 6000.f) * WarpStrength;
        pp = Warp3D(pp, FMath::Max(2000.f, S.WarpPatchCm), WarpAmp);
    }

    // =========================
    // 4) 표면 굴곡
    // =========================
    const float DetailScale = FMath::Max(800.f, S.DetailScaleCm);
    const FVector dp = pp / DetailScale;

    const float n = FBM3D(dp, 4, 2.0f, 0.52f);

    const float RoughAmp = FMath::Clamp(S.BaseField3DStrengthCm, 1500.f, 9000.f);

    density += n * RoughAmp * faceMask * z01;

    // =========================
    // 5) 오버행
    // =========================
    const float VolumeStrength = FMath::Clamp(S.VolumeStrength, 0.f, 1.8f);
    if (VolumeStrength > 0.f)
    {
        const float OverScale = FMath::Max(1200.f, S.OverhangScaleCm);
        const FVector op = pp / OverScale;

        float r01 = RidgedFBM01(op, 4, 2.0f, 0.55f);

        const float bias = FMath::Clamp(S.OverhangBias, 0.50f, 0.72f);

        float m = (r01 - bias) / FMath::Max(0.0001f, (1.f - bias));
        m = Clamp01(m);
        m = FMath::Pow(m, 2.0f);

        const float depth = FMath::Clamp(S.OverhangDepthCm, 300.f, 6000.f);

        density += m * depth * VolumeStrength * faceMask * z01;
    }

    // =========================
    // 6) 동굴
    // =========================
    const float CaveStrength = FMath::Clamp(S.CaveStrength, 0.f, 1.4f);
    if (CaveStrength > 0.f)
    {
        const float CaveScale = FMath::Max(1500.f, S.CaveScaleCm);
        const FVector cp = pp / CaveScale;

        const float c = FBM3D(cp, 5, 2.0f, 0.5f);      // -1..1
        const float c01 = c * 0.5f + 0.5f;             // 0..1

        const float thr = FMath::Clamp(S.CaveThreshold, 0.55f, 0.75f);
        const float band = FMath::Clamp(S.CaveBand, 0.06f, 0.22f);

        float pocket = (c01 - thr) / FMath::Max(0.0001f, band);
        pocket = Clamp01(pocket);
        pocket = FMath::Pow(pocket, 1.8f);

        const float pocketDepth = FMath::Clamp(S.CaveDepthCm, 200.f, 2500.f);

        const float midBand = SmoothStep(0.25f, 0.55f, z01) * (1.f - SmoothStep(0.70f, 0.95f, z01));

        density -= pocket * pocketDepth * CaveStrength * faceMask * midBand;
    }

    return density;
}