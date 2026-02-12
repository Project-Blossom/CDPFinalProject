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
        const float n = Noise3D(p * freq);
        sum += Ridged01(n) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return Clamp01(sum);
}

FVector FVoxelDensityGenerator::Warp3D(const FVector& p, float PatchCm, float AmpCm) const
{
    const float inv = 1.f / FMath::Max(1.f, PatchCm);
    const FVector q = p * inv;

    const float nx = FBM3D(q + FVector(13,  7,  5), 3, 2.f, 0.5f);
    const float ny = FBM3D(q + FVector( 5, 11, 17), 3, 2.f, 0.5f);
    const float nz = FBM3D(q + FVector(19,  3, 29), 3, 2.f, 0.5f);

    return p + FVector(nx, ny, nz) * AmpCm;
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPosCm) const
{
    const float Iso = S.IsoLevel;

    // Local (cm)
    const FVector Local = WorldPosCm - TerrainOriginWorld;
    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    // -------------------------
    // 1) Cliff Base
    // -------------------------
    const float CliffHalfW = FMath::Max(500.f, S.CliffHalfWidthCm);
    const float CliffH = FMath::Max(500.f, S.CliffHeightCm);
    const float CliffT = FMath::Max(200.f, S.CliffThicknessCm);

    const FVector CliffCenter(CliffT * 0.5f, 0.f, S.BaseHeightCm + CliffH * 0.5f);
    const FVector CliffHalfExtents(CliffT * 0.5f, CliffHalfW, CliffH * 0.5f);

    const FVector Pc = Local - CliffCenter;

    // -------------------------
    // 2) 빈 공간 마킹
    // -------------------------
    const float Influence =
        FMath::Max(
            FMath::Max(S.CliffSurfaceAmpCm, 0.f),
            FMath::Max(S.OverhangDepthCm, 0.f)
        )
        + FMath::Max(S.WarpAmpCm, 0.f)
        + FMath::Max(S.BaseField3DStrengthCm, 0.f)
        + Voxel * 8.f;

    if (S.bUseCliffBase)
    {
        if (FMath::Abs(Pc.X) > (CliffHalfExtents.X + Influence) ||
            FMath::Abs(Pc.Y) > (CliffHalfExtents.Y + Influence) ||
            FMath::Abs(Pc.Z) > (CliffHalfExtents.Z + Influence))
        {
            return Iso - 1.0f;
        }
    }

    // -------------------------
    // 3) 베이스 density
    // -------------------------
    float density = 0.f;

    if (S.bUseCliffBase)
    {
        density = -SdBox(Pc, CliffHalfExtents);

        if (S.CliffSurfaceAmpCm > 0.f)
        {
            const float Scale = FMath::Max(300.f, S.CliffSurfaceScaleCm);
            const FVector pp = SeededDomain(Local) / Scale;
            const float n = FBM3D(pp, 4, 2.0f, 0.55f);
            density += n * S.CliffSurfaceAmpCm;
        }
    }
    else
    {
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

        density = -SdBox(P, HalfExtents);

        if (density < -Voxel * 6.f)
            return Iso - 1.0f; // air
    }

    // -------------------------
    // 4) faceMask
    // -------------------------
    float faceMask = 0.f;

    if (S.bUseCliffBase)
    {
        const float FaceDist = (CliffHalfExtents.X - Pc.X);
        const float FaceFade = FMath::Max(800.f, S.OverhangFadeCm * 0.35f);

        if (FaceDist > 0.f)
            faceMask = 1.f - SmoothStep(0.f, FaceFade, FaceDist);
    }
    else
    {
        faceMask = 1.f;
    }

    const float zMin = S.BaseHeightCm + 500.f;
    const float zMax = S.BaseHeightCm + (S.bUseCliffBase ? CliffH : (S.ChunkZ * Voxel)) - 800.f;
    const float z01 = Remap01(Local.Z, zMin, zMax);

    // -------------------------
    // 5) 도메인 워프
    // -------------------------
    FVector pp = SeededDomain(Local);

    const float WarpStrength = FMath::Clamp(S.WarpStrength, 0.f, 1.2f);
    if (WarpStrength > 0.f && S.WarpAmpCm > 0.f && S.WarpPatchCm > 1.f)
    {
        const float WarpAmp = FMath::Clamp(S.WarpAmpCm, 0.f, 6000.f) * WarpStrength;
        pp = Warp3D(pp, FMath::Max(2000.f, S.WarpPatchCm), WarpAmp);
    }

    // -------------------------
    // 6) 표면 굴곡
    // -------------------------
    const float DetailScale = FMath::Max(800.f, S.DetailScaleCm);
    const float n = FBM3D(pp / DetailScale, 4, 2.0f, 0.52f);
    const float RoughAmp = FMath::Clamp(S.BaseField3DStrengthCm, 1500.f, 9000.f);

    density += n * RoughAmp * faceMask * z01;

    // -------------------------
    // 7) 오버행
    // -------------------------
    const float VolumeStrength = FMath::Clamp(S.VolumeStrength, 0.f, 1.8f);
    if (VolumeStrength > 0.f)
    {
        const float OverScale = FMath::Max(1200.f, S.OverhangScaleCm);
        const float r01 = RidgedFBM01(pp / OverScale, 4, 2.0f, 0.55f);

        const float bias = FMath::Clamp(S.OverhangBias, 0.50f, 0.72f);

        float m = (r01 - bias) / FMath::Max(0.0001f, (1.f - bias));
        m = Clamp01(m);
        m = FMath::Pow(m, 2.0f);

        const float depth = FMath::Clamp(S.OverhangDepthCm, 300.f, 6000.f);
        density += m * depth * VolumeStrength * faceMask * z01;
    }

    return density;
}