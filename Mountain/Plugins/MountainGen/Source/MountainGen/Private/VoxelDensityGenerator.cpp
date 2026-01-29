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

    // UE 기본 Perlin ( -1..1 )
    static FORCEINLINE float Noise3D(const FVector& p)
    {
        return FMath::PerlinNoise3D(p);
    }
    
    static FORCEINLINE float Ridged01(float n)
    {
        // 1 - abs(noise) : ridged (0..1)
        float r = 1.f - FMath::Abs(n);
        r = FMath::Clamp(r, 0.f, 1.f);
        return r * r;
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

    const FVector Local = WorldPosCm - TerrainOriginWorld;

    // ============================================================
    // 1) 상단 캡 / 하단 솔리드
    // ============================================================
    const float BaseZWorld = TerrainOriginWorld.Z + S.BaseHeightCm;
    const float TopZWorld = BaseZWorld + FMath::Max(1000.f, S.HeightAmpCm);

    if (WorldPosCm.Z > TopZWorld)          return Iso - 1.0f;
    if (WorldPosCm.Z < BaseZWorld - 2000.f) return Iso + 1.0f;

    // ============================================================
    // 2) Envelope
    // ============================================================
    const FVector2D d2(Local.X, Local.Y);
    const float r = d2.Size();

    const float RadiusCm = FMath::Max(1000.f, S.EnvelopeRadiusCm);
    const float EdgeFadeCm = FMath::Max(100.f, S.EnvelopeEdgeFadeCm);

    const float edge01 = SmoothStep(RadiusCm - EdgeFadeCm, RadiusCm, r); // 0..1
    const float edgeCut = -edge01 * FMath::Max(5000.f, S.EnvelopeEdgeCutCm);

    // ============================================================
    // 3) 도메인 워프 + 3D 프랙탈 필드
    // ============================================================
    FVector p = Local;
    const FVector so = SeedOffset();
    p += so;

    if (S.WarpStrength > 0.f && S.WarpAmpCm > 0.f && S.WarpPatchCm > 1.f)
    {
        const float WarpAmp = S.WarpAmpCm * S.WarpStrength;
        p = Warp3D(p, S.WarpPatchCm, WarpAmp);
    }

    const float BaseScale = FMath::Max(100.f, S.BaseField3DScaleCm);
    const FVector bp = p / BaseScale;

    float rid01 = RidgedFBM01(bp, FMath::Clamp(S.BaseField3DOctaves, 1, 8), 2.0f, 0.55f);
    rid01 = FMath::Pow(Clamp01(rid01), FMath::Max(1.f, S.BaseField3DRidgedPower));
    const float ridSigned = rid01 * 2.f - 1.f;

    const float DetailScale = FMath::Max(300.f, S.DetailScaleCm);
    const FVector dp = p / DetailScale;
    const float mid = FBM3D(dp, 4, 2.15f, 0.52f);

    const float MassAmp = FMath::Max(1000.f, S.BaseField3DStrengthCm);
    float field = ridSigned * MassAmp + mid * (0.35f * MassAmp);

    // ============================================================
    // 4) Z 게이팅
    // ============================================================
    const float Gravity = FMath::Max(0.01f, S.GravityStrength) * FMath::Max(0.1f, S.GravityScale);

    const float h01 = Clamp01((WorldPosCm.Z - BaseZWorld) / FMath::Max(1.f, (TopZWorld - BaseZWorld)));

    const float bulkSolid = (1.0f - h01) * (MassAmp * 0.90f);

    const float softCap01 = SmoothStep(TopZWorld - 4000.f, TopZWorld + 2000.f, WorldPosCm.Z);
    const float capCut = -softCap01 * (MassAmp * 2.0f);

    const float vertical = (BaseZWorld - WorldPosCm.Z) * Gravity;

    field *= FMath::Lerp(1.0f, 0.35f, h01);

    float density = vertical + bulkSolid + field + edgeCut + capCut;

    // ============================================================
    // 5) Overhang
    // ============================================================
    if (S.VolumeStrength > 0.f && S.OverhangScaleCm > 1.f)
    {
        const float OverScale = FMath::Max(1.f, S.OverhangScaleCm);
        const FVector op = (p / OverScale);

        float n01 = RidgedFBM01(op, 4, 2.0f, 0.55f);

        float mask = (n01 - S.OverhangBias) / FMath::Max(0.0001f, (1.f - S.OverhangBias));
        mask = Clamp01(mask);
        mask = FMath::Pow(mask, 2.2f);

        const float oh01 = SmoothStep(BaseZWorld + 2000.f, BaseZWorld + FMath::Max(1.f, S.OverhangFadeCm), WorldPosCm.Z);
        density += mask * oh01 * S.VolumeStrength * S.OverhangDepthCm;
    }

    // ============================================================
    // 6) Caves
    // ============================================================
    if (S.CaveStrength > 0.f && S.CaveScaleCm > 1.f)
    {
        const float zForCave = (S.bCaveHeightsAreAbsoluteWorldZ ? WorldPosCm.Z : Local.Z);

        const float hUp = SmoothStep(S.CaveMinHeightCm, S.CaveMinHeightCm + 4000.f, zForCave);
        const float hDown = 1.f - SmoothStep(S.CaveMaxHeightCm, S.CaveMaxHeightCm + 4000.f, zForCave);
        const float HeightBand = hUp * hDown;

        if (HeightBand > 0.001f)
        {
            const float CaveScale = FMath::Max(1.f, S.CaveScaleCm);
            const FVector cp = p / CaveScale;

            const float c = FBM3D(cp, 5, 2.0f, 0.5f);
            const float c01 = c * 0.5f + 0.5f;

            float m = (c01 - S.CaveThreshold) / FMath::Max(0.0001f, S.CaveBand);
            m = Clamp01(m);
            m = FMath::Pow(m, 1.8f);

            density -= m * S.CaveStrength * HeightBand * S.CaveDepthCm;
        }
    }

    return density;
}