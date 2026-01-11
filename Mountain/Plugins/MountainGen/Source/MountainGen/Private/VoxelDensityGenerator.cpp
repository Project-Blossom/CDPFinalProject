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

// 2D fBm: 2D 노이즈를 여러 옥타브로 합성해서 “지형 높이” 같은 큰 흐름을 만든다.
static FORCEINLINE float FBM2D(float x, float y, int32 Octaves, float Lacunarity, float Gain)
{
    float sum = 0.0f, amp = 1.0f, freq = 1.0f;
    for (int i = 0; i < Octaves; ++i)
    {
        sum += Noise2D(x * freq, y * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum; // 대략 [-?, +?]
}

// 3D fBm: 3D 공간에서 노이즈를 합성해서 “오버행/언더컷/볼륨감”을 만들 때 쓴다.
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

// Ridged: 절벽처럼 “날카로운 능선/포켓”이 생기도록 노이즈를 변형한다.
static FORCEINLINE float Ridged(float n)
{
    // n: -1..1  ->  ridge: 0..1
    return 1.0f - FMath::Abs(n);
}

float FVoxelDensityGenerator::GetDensity(int32 X, int32 Y, int32 Z) const
{
    const float wx = X * VoxelSizeCm;
    const float wy = Y * VoxelSizeCm;
    const float wz = Z * VoxelSizeCm;

    const float S = SeedOffsetCm();

    const float WorldFreq = 1.0f / FMath::Max(1.0f, WorldScaleCm);
    const float DetailFreq = 1.0f / FMath::Max(1.0f, DetailScaleCm);
    const float WarpFreq = 1.0f / FMath::Max(1.0f, WarpPatchCm);
    const float CaveFreq = 1.0f / FMath::Max(1.0f, CaveScaleCm);

    // ------------------------------------------------------------
    // (1) 2D 기반 높이 지형(산의 기본 실루엣)
    //     - 여기서 “일반적인 산/절벽 형태(표면)”가 결정된다.
    // ------------------------------------------------------------
    const float h0 = FBM2D((wx + S) * WorldFreq, (wy + S) * WorldFreq, 5, 2.0f, 0.5f);
    const float h1 = FBM2D((wx - S) * DetailFreq, (wy + S) * DetailFreq, 4, 2.2f, 0.55f);

    // X 방향으로 끝쪽이 더 높아지는 램프(진행 방향 고도 상승)
    const float RampT = FMath::Clamp(wx / FMath::Max(1.0f, RampLengthCm), 0.0f, 1.0f);
    const float Ramp = RampT * RampHeightCm;

    const float SurfaceHeight =
        BaseHeightCm
        + Ramp
        + (h0 * HeightAmpCm * 0.55f)
        + (h1 * HeightAmpCm * 0.18f);

    // density 정의:
    //   density < 0 : Solid(암벽/지형 내부)
    //   density > 0 : Air(빈 공간)
    float density = (wz - SurfaceHeight);

    // 지표 근처에서 아주 약하게 안정화(떠 있는 레이어 방지용)
    // 원치 않으면 계수를 0으로 두면 된다.
    density -= (1.0f - SmoothStep(0.0f, 5000.0f, (wz - SurfaceHeight))) * (GravityStrength * 0.15f);

    // ------------------------------------------------------------
    // (2) Domain Warp(선택): 오버행/동굴에 쓸 좌표를 살짝 뒤틀어서
    //     반복 패턴 느낌을 줄이고 자연스러운 찢김/휘어짐을 만든다.
    // ------------------------------------------------------------
    float wx2 = wx, wy2 = wy, wz2 = wz;
    if (WarpStrength > 0.0f && WarpAmpCm > 0.0f)
    {
        const float W = WarpAmpCm * WarpStrength;

        const float wnx = Noise3D((wx + S) * WarpFreq, (wy + S) * WarpFreq, 0.0f);
        const float wny = Noise3D((wx - S) * WarpFreq, (wy + S) * WarpFreq, 0.0f);
        const float wnz = Noise3D((wx + S) * WarpFreq, (wy - S) * WarpFreq, 0.0f);

        wx2 = wx + wnx * W;
        wy2 = wy + wny * W;
        wz2 = wz + wnz * (W * 0.25f);
    }

    // ------------------------------------------------------------
    // (3) Overhang(언더컷/절벽) 생성
    //     핵심 제약:
    //     - "표면 근처에서만" 작동하도록 NearSurface로 제한
    //     - 이걸 안 하면 공중 섬/뒤집힌 구조가 쉽게 생긴다.
    // ------------------------------------------------------------
    const float Above = (wz - SurfaceHeight); // 표면 위면 +, 표면 아래면 -
    const float NearSurface = 1.0f - SmoothStep(0.0f, FMath::Max(1.0f, OverhangFadeCm), FMath::Abs(Above));
    // NearSurface: 표면에서 1, 멀어질수록 0

    if (VolumeStrength > 0.0f && NearSurface > 0.001f)
    {
        // 3D Ridged fBm으로 절벽 포켓/언더컷 느낌 만들기
        const float n = FBM3D((wx2 + S) * WorldFreq, (wy2 + S) * WorldFreq, (wz2 + S) * WorldFreq,
            3, 2.0f, 0.5f);
        const float r = Ridged(n); // 0..1

        // OverhangBias를 넘는 강한 ridge만 선택
        float ridgeMask = (r - OverhangBias) / FMath::Max(0.0001f, (1.0f - OverhangBias));
        ridgeMask = FMath::Clamp(ridgeMask, 0.0f, 1.0f);
        ridgeMask = ridgeMask * ridgeMask * (3.0f - 2.0f * ridgeMask);

        // carve는 “공기”를 만드는 방향이므로 density를 +로 올린다.
        const float carve = ridgeMask * NearSurface * VolumeStrength;

        // VoxelSize 변화에 덜 민감하도록 스케일 보정
        density += carve * (OverhangDepthCm / FMath::Max(1.0f, VoxelSizeCm));
    }

    // ------------------------------------------------------------
    // (4) 얕은 동굴(Alcove) 생성
    //     목표:
    //     - “깊은 지하 동굴”이 아니라,
    //       사람이 설 수 있는 ‘중간 공간/굴턱’ 같은 포켓을 만들기
    //     - 그래서 (a) 특정 높이 밴드, (b) 표면 근처로 제한한다.
    // ------------------------------------------------------------
    if (CaveStrength > 0.0f)
    {
        // 높이 밴드 마스크(0..1)
        const float hMaskUp = SmoothStep(CaveMinHeightCm, CaveMinHeightCm + 4000.0f, wz);
        const float hMaskDown = 1.0f - SmoothStep(CaveMaxHeightCm, CaveMaxHeightCm + 4000.0f, wz);
        const float HeightBand = hMaskUp * hMaskDown;

        // 표면 근처 제한(너무 내부에 “스위스 치즈”처럼 뚫리지 않게)
        const float CaveNearSurface = 1.0f - SmoothStep(0.0f, 2500.0f, FMath::Abs(Above));

        if (HeightBand > 0.001f && CaveNearSurface > 0.001f)
        {
            const float c = Noise3D((wx2 + S) * CaveFreq, (wy2 - S) * CaveFreq, (wz2 + S) * CaveFreq); // -1..1
            const float c01 = (c * 0.5f + 0.5f); // 0..1로 변환

            float mask = (c01 - CaveThreshold) / FMath::Max(0.0001f, CaveBand);
            mask = FMath::Clamp(mask, 0.0f, 1.0f);
            mask = mask * mask * (3.0f - 2.0f * mask);

            // 동굴(포켓) => 공기 생성 => density를 +로
            density += mask * CaveStrength * HeightBand * CaveNearSurface * 2.0f;
        }
    }

    return density;
}