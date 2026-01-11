#pragma once
#include "CoreMinimal.h"

struct FVoxelDensityGenerator
{
    int32 Seed = 1557;

    // 복셀 좌표 -> 월드(cm) 변환 스케일
    float VoxelSizeCm = 100.0f;

    // ---------- 큰 형태(지형 덩어리) ----------
    float WorldScaleCm = 24000.0f;   // 큰 지형 형태
    float DetailScaleCm = 6000.0f;   // 중간 디테일

    // ---------- 높이 / 경사 ----------
    float BaseHeightCm = 0.0f;       // 기준 지표 높이
    float HeightAmpCm = 30000.0f;    // 지형 기복(예: 300m)
    float GravityStrength = 1.0f;    // 지표가 “떠 보이지 않게” 안정화(부드럽게)

    // ---------- Ramp: "끝쪽이 더 높아지게" ----------
    float RampLengthCm = 60000.0f;   // X방향 램프 길이
    float RampHeightCm = 45000.0f;   // 끝까지 갔을 때 추가 높이(+450m)

    // ---------- Overhang(언더컷/절벽) ----------
    float VolumeStrength = 1.0f;
    float OverhangFadeCm = 2500.0f;  // Overhang 효과를 표면 근처에서만 적용하는 두께
    float OverhangDepthCm = 2500.0f; // 표면 근처에서 안쪽/바깥쪽으로 파고드는 정도
    float OverhangBias = 0.65f;      // 임계값(높을수록 드물지만 강하게 생김)

    // ---------- Domain Warp(뒤틀림) ----------
    float WarpPatchCm = 12000.0f;
    float WarpAmpCm = 800.0f;
    float WarpStrength = 1.0f;

    // ---------- 얕은 동굴(Alcove, 포켓 형태) ----------
    float CaveScaleCm = 2600.0f;
    float CaveThreshold = 0.35f;     // 낮을수록 동굴이 많이 생김
    float CaveBand = 0.25f;          // 경계 스무딩 폭
    float CaveStrength = 1.0f;

    // 동굴이 “지하 전체”가 아니라 특정 높이 구간에서만 나오게 하는 밴드
    float CaveMinHeightCm = 6000.0f;   // 이 높이 이상부터 동굴 생성 시작
    float CaveMaxHeightCm = 22000.0f;  // 이 높이 이상에서는 동굴 생성 감소/종료

    explicit FVoxelDensityGenerator(int32 InSeed) : Seed(InSeed) {}

    FORCEINLINE float SeedOffsetCm() const
    {
        return (float)Seed * 10000.0f;
    }

    float GetDensity(int32 X, int32 Y, int32 Z) const;
};