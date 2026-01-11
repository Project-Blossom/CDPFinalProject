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

// 2D fBm
static FORCEINLINE float FBM2D(float x, float y, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f;
    float amp = 1.0f;
    float freq = 1.0f;

    for (int32 i = 0; i < Octaves; ++i)
    {
        sum += Noise2D(x * freq, y * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum; // 대략 [-something, +something]
}

// 3D fBm
static FORCEINLINE float FBM3D(float x, float y, float z, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f;
    float amp = 1.0f;
    float freq = 1.0f;

    for (int32 i = 0; i < Octaves; ++i)
    {
        sum += Noise3D(x * freq, y * freq, z * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum;
}

float FVoxelDensityGenerator::GetDensity(int32 X, int32 Y, int32 Z) const
{
    // 월드 좌표(cm)
    const float wx = X * VoxelSizeCm;
    const float wy = Y * VoxelSizeCm;
    const float wz = Z * VoxelSizeCm;

    const float S = SeedOffsetCm();

    const float WorldFreq = 1.0f / FMath::Max(1.0f, WorldScaleCm);
    const float DetailFreq = 1.0f / FMath::Max(1.0f, DetailScaleCm);
    const float WarpFreq = 1.0f / FMath::Max(1.0f, WarpPatchCm);
    const float CaveFreq = 1.0f / FMath::Max(1.0f, CaveScaleCm);

    // =========================
    // (A) 2D 프랙탈로 지형 표면 높이 생성
    // =========================
    const float h0 = FBM2D((wx + S) * WorldFreq, (wy + S) * WorldFreq, 5, 2.0f, 0.5f);
    const float h1 = FBM2D((wx - S) * DetailFreq, (wy + S) * DetailFreq, 4, 2.2f, 0.55f);

    // 램프(끝이 더 높아지게): +X 방향으로 BaseHeight + RampHeight까지
    const float RampT = FMath::Clamp(wx / FMath::Max(1.0f, RampLengthCm), 0.0f, 1.0f);
    const float Ramp = RampT * RampHeightCm;

    const float SurfaceHeight =
        BaseHeightCm
        + Ramp
        + (h0 * HeightAmpCm * 0.55f)
        + (h1 * HeightAmpCm * 0.18f);

    // =========================
    // (B) 기본 density: 아래는 음수(solid), 위는 양수(air)
    // =========================
    float density = (wz - SurfaceHeight);

    // =========================
    // (C) Domain Warp + 3D 볼륨(오버행 느낌)
    //     단, 표면 근처에서만 적용(섬 제거 핵심)
    // =========================
    float wx2 = wx, wy2 = wy, wz2 = wz;

    if (WarpStrength > 0.0f && WarpAmpCm > 0.0f)
    {
        const float W = WarpAmpCm * WarpStrength;

        const float wnx = Noise3D((wx + S) * WarpFreq, (wy + S) * WarpFreq, 0.0f);
        const float wny = Noise3D((wx - S) * WarpFreq, (wy + S) * WarpFreq, 0.0f);
        const float wnz = Noise3D((wx + S) * WarpFreq, (wy - S) * WarpFreq, 0.0f);

        wx2 = wx + wnx * W;
        wy2 = wy + wny * W;
        wz2 = wz + wnz * (W * 0.35f);
    }

    // 표면 위/아래 거리
    const float Above = (wz - SurfaceHeight);

    // 표면에서의 절댓거리(위든 아래든)
    const float DistToSurface = FMath::Abs(Above);

    // 표면에서 멀어질수록 3D 볼륨 영향 0으로 페이드
    const float VolumeFade =
        1.0f - SmoothStep(0.0f, FMath::Max(1.0f, OverhangFadeCm), DistToSurface);

    if (VolumeStrength > 0.0f && VolumeFade > 0.001f)
    {
        const float v = FBM3D((wx2 + S) * WorldFreq, (wy2 + S) * WorldFreq, (wz2 + S) * WorldFreq,
            3, 2.0f, 0.5f);

        density -= v * (HeightAmpCm * 0.08f) * VolumeStrength * VolumeFade;
    }

    // =========================
    // (D) Caves (공기 생성: density를 +로 올림)
    // =========================
    if (CaveStrength > 0.0f)
    {
        const float cave = Noise3D((wx2 + S) * CaveFreq, (wy2 - S) * CaveFreq, (wz2 + S) * CaveFreq);
        const float mask = cave - CaveThreshold;

        float carve = FMath::Clamp(mask / FMath::Max(0.0001f, CaveBand), 0.0f, 1.0f);
        carve = carve * carve * (3.0f - 2.0f * carve);

        density += carve * CaveStrength * VolumeFade; // 표면 근처에 동굴 집중
    }

    return density;
}