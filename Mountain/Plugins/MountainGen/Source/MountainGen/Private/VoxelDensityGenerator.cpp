#include "VoxelDensityGenerator.h"

static FORCEINLINE float Noise3D(float x, float y, float z)
{
    return FMath::PerlinNoise3D(FVector(x, y, z)); // -1 ~ 1
}

static FORCEINLINE float Noise2D(float x, float y)
{
    return FMath::PerlinNoise2D(FVector2D(x, y)); // -1 ~ 1
}

static FORCEINLINE float SmoothStep(float edge0, float edge1, float x)
{
    float t = FMath::Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float FVoxelDensityGenerator::GetDensity(int32 X, int32 Y, int32 Z) const
{
    // ✅ 여기 핵심: 100 고정이 아니라 VoxelSizeCm 사용
    const float wx = X * VoxelSizeCm;
    const float wy = Y * VoxelSizeCm;
    const float wz = Z * VoxelSizeCm;

    const float S = SeedOffsetCm();

    // Scale(cm) -> Freq
    const float WorldFreq = 1.0f / FMath::Max(1.0f, WorldScaleCm);
    const float DetailFreq = 1.0f / FMath::Max(1.0f, DetailScaleCm);
    const float CaveFreq = 1.0f / FMath::Max(1.0f, CaveScaleCm);

    // 베이스: 위로 갈수록 공기(+)
    float base = (wz - HeightAmp);

    float n1 = Noise3D((wx + S) * WorldFreq, (wy + S) * WorldFreq, (wz + S) * WorldFreq);
    float n2 = Noise3D((wx - S) * DetailFreq, (wy + S) * DetailFreq, (wz + S) * DetailFreq);

    float density =
        base
        + (n1 * (HeightAmp * 0.13f))
        + (n2 * (HeightAmp * 0.04f))
        + BaseBias;

    // =========================
    // 절벽 마스크
    // =========================
    const float CliffPatchCm = FMath::Max(3000.0f, WorldScaleCm * 0.5f);
    const float CliffFreq = 1.0f / CliffPatchCm;

    float ridge = FMath::Abs(Noise2D((wx + S) * CliffFreq, (wy - S) * CliffFreq));
    float cliffMask = SmoothStep(0.55f, 0.85f, ridge);

    // =========================
    // Domain Warp (오버행)
    // =========================
    const float OverhangDepthCm = 1000.0f; // 10m
    const float WarpPatchCm = FMath::Max(6000.0f, WorldScaleCm * 0.5f);
    const float WarpFreq = 1.0f / WarpPatchCm;

    const float WarpAmpWorld = OverhangDepthCm * OverhangAmp * cliffMask;

    float wx2 = wx + Noise3D((wx + S) * WarpFreq, (wy + S) * WarpFreq, (wz + S) * WarpFreq) * WarpAmpWorld;
    float wy2 = wy + Noise3D((wx - S) * WarpFreq, (wy + S) * WarpFreq, (wz + S) * WarpFreq) * WarpAmpWorld;
    float wz2 = wz + Noise3D((wx + S) * WarpFreq, (wy - S) * WarpFreq, (wz + S) * WarpFreq) * (WarpAmpWorld * 0.35f);

    // =========================
    // 동굴 1: 양수만 carve
    // =========================
    float cave = Noise3D((wx2 + S) * CaveFreq, (wy2 - S) * CaveFreq, (wz2 + S) * CaveFreq);
    float caveMask = cave - CaveThreshold;

    const float Band = 0.25f;
    float carve = FMath::Clamp(caveMask / Band, 0.0f, 1.0f);
    carve = carve * carve * (3.0f - 2.0f * carve);

    const float CaveStrength = HeightAmp * 0.22f * CaveAmp;
    density -= carve * CaveStrength * (0.15f + 0.85f * cliffMask);

    // =========================
    // 동굴 2: 양수만 carve
    // =========================
    float cave2 = Noise3D((wx2 + S) * (CaveFreq * 2.2f), (wy2 - S) * (CaveFreq * 2.2f), (wz2 + S) * (CaveFreq * 2.2f));
    float cave2Mask = cave2 - (CaveThreshold + 0.12f);

    const float Band2 = 0.20f;
    float carve2 = FMath::Clamp(cave2Mask / Band2, 0.0f, 1.0f);
    carve2 = carve2 * carve2 * (3.0f - 2.0f * carve2);

    density -= carve2 * (HeightAmp * 0.10f) * cliffMask;

    return -density;
}