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
    // 1) 월드 스케일
    const float wx = X * 100.0f;
    const float wy = Y * 100.0f;
    const float wz = Z * 100.0f;

    // ------------------------------------------------------------
    // 2) 베이스 지형
    // ------------------------------------------------------------
    // 표면이 청크 범위 안에 오도록: (wz - HeightAmp)
    float base = (wz - HeightAmp);

    float n1 = Noise3D(wx * WorldFreq, wy * WorldFreq, wz * WorldFreq);
    float n2 = Noise3D(wx * DetailFreq, wy * DetailFreq, wz * DetailFreq);

    float density = base
        + (n1 * (HeightAmp * 0.13f))
        + (n2 * (HeightAmp * 0.04f))
        + BaseBias;

    // ============================================================
    // 3 3D: 동굴/오버행이 강제하는 파트
    // ============================================================

    // ------------------------------------------------------------
    // 3-A) 절벽/바위 지대 마스크 (너무 약하면 티가 안 나니 범위를 넓힘)
    // ------------------------------------------------------------
    // ridge가 0~1 근처로 나오도록 abs 후 정규화
    float ridge = FMath::Abs(Noise2D(wx * 0.0009f, wy * 0.0009f));  // 느리게 변화
    float cliffMask = SmoothStep(0.35f, 0.85f, ridge);             // 넓게 잡아서 "무조건 보이게"

    // ------------------------------------------------------------
    // 3-B) 3D Domain Warp (오버행 핵심)
    //      좌표 자체를 비틀면 z 단조성이 크게 깨져서 오버행이 확 살아남
    // ------------------------------------------------------------
    // OverhangAmp(0~1)를 진짜 체감되게 월드 단위 워프 크기로 변환
    const float WarpFreq = FMath::Max(0.0015f, CaveFreq * 0.25f);   // CaveFreq가 크면 자동으로 낮춰줌
    const float WarpAmpWorld = (1500.0f * OverhangAmp) * cliffMask; // 0~1500cm 정도

    float wx2 = wx + Noise3D(wx * WarpFreq, wy * WarpFreq, wz * WarpFreq) * WarpAmpWorld;
    float wy2 = wy + Noise3D((wx + 37.0f) * WarpFreq, (wy + 91.0f) * WarpFreq, (wz + 17.0f) * WarpFreq) * WarpAmpWorld;
    float wz2 = wz + Noise3D((wx - 53.0f) * WarpFreq, (wy + 19.0f) * WarpFreq, (wz + 73.0f) * WarpFreq) * (WarpAmpWorld * 0.35f);

    // ------------------------------------------------------------
    // 3-C) 동굴: 3D 노이즈 "임계값"으로 파내기 (가장 직관적으로 동굴이 생김)
    // ------------------------------------------------------------
    // CaveFreq가 너무 크면 동굴이 쌀알처럼 되어 티가 안 나니, 자동 보정
    const float CaveFreqWorld = FMath::Clamp(CaveFreq, 0.003f, 0.02f);

    float cave = Noise3D(wx2 * CaveFreqWorld, wy2 * CaveFreqWorld, wz2 * CaveFreqWorld); // -1~1

    // cave > threshold 인 곳을 공기로 만들고 싶으면, density에 큰 값을 더/빼서 강제로 표면을 찢는다.
    // 여기서는 caveMask가 양수인 곳을 "강하게 파냄"
    float caveMask = cave - CaveThreshold;

    // "눈에 띄게" 만들기 위해 강도는 HeightAmp 기반으로 크게
    const float CaveStrength = HeightAmp * 0.35f * CaveAmp; // HeightAmp=3000이면 1050

    // 동굴은 cliffMask에서 더 강하게, 평지에서도 약간은 작동하게(입구가 보이게)
    density -= caveMask * CaveStrength * (0.35f + 0.65f * cliffMask);

    // ------------------------------------------------------------
    // 3-D) 추가: 동굴을 더 '텅 빈 공간'으로 만드는 두 번째 노이즈
    // ------------------------------------------------------------
    float cave2 = Noise3D(wx2 * (CaveFreqWorld * 2.2f), wy2 * (CaveFreqWorld * 2.2f), wz2 * (CaveFreqWorld * 2.2f));
    float cave2Mask = cave2 - (CaveThreshold + 0.15f);
    density -= cave2Mask * (HeightAmp * 0.12f) * cliffMask;

    // 4) 부호
    return -density;
}