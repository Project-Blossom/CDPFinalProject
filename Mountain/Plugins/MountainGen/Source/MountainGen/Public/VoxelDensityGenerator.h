#pragma once
#include "CoreMinimal.h"

struct FVoxelDensityGenerator
{
    int32 Seed = 1557;

    float VoxelSizeCm = 100.0f;

    // =========================
    // Scale Params (cm 단위 "크기")
    // 값이 클수록 더 큰 덩어리/부드러움
    // =========================
    float WorldScaleCm = 20000.0f; // 200m: 큰 지형 덩어리(절벽/산맥)
    float DetailScaleCm = 3000.0f;  // 30m : 중간 디테일
    float CaveScaleCm = 1800.0f;  // 18m : 동굴 덩어리

    // =========================
    // Amplitude Params
    // =========================
    float HeightAmp = 3000.0f;
    float OverhangAmp = 0.6f;
    float CaveAmp = 1.0f;

    // =========================
    // Cave Params
    // =========================
    float CaveThreshold = 0.55f; // 높을수록 동굴이 드물어짐
    float BaseBias = 10.0f; // 전체 밀도 이동

    explicit FVoxelDensityGenerator(int32 InSeed) : Seed(InSeed) {}

    FORCEINLINE float SeedOffsetCm() const
    {
        // Seed가 바뀌면 노이즈 입력 좌표가 크게 이동 -> 확실히 다른 지형
        return (float)Seed * 100000.0f; // 1000m 단위 점프
    }

    float GetDensity(int32 X, int32 Y, int32 Z) const;
};