#pragma once
#include "CoreMinimal.h"

struct FVoxelDensityGenerator
{
    int32 Seed = 1557;

    // Voxel -> World (cm)
    float VoxelSizeCm = 100.0f;

    // =========================
    // Surface (지형) 프랙탈 스케일
    // =========================
    float WorldScaleCm = 24000.0f; // 큰 지형 덩어리
    float DetailScaleCm = 6000.0f;  // 중간 디테일

    // 지형 기준 높이 / 기복 (cm)
    float BaseHeightCm = 0.0f;
    float HeightAmpCm = 30000.0f; // 300m

    // "끝 지점이 더 높아지게" 램프(경사) 추가
    // Chunk의 +X 방향으로 높아짐
    float RampHeightCm = 20000.0f; // 200m 추가 상승
    float RampLengthCm = 6400.0f;  // 이 길이(cm) 동안 RampHeightCm까지 상승

    // =========================
    // 3D 볼륨(오버행) - 단, "지표 근처"에서만 영향 주게(섬 방지)
    // =========================
    float VolumeStrength = 0.35f;     // 0~1 (너무 크면 섬 생김)
    float OverhangFadeCm = 3000.0f;   // 지표 위로 이 높이 이상이면 3D 볼륨 영향 거의 0

    // Domain warp
    float WarpPatchCm = 12000.0f;
    float WarpAmpCm = 800.0f;
    float WarpStrength = 1.0f;

    // =========================
    // Caves
    // =========================
    float CaveScaleCm = 2200.0f;
    float CaveThreshold = 0.55f;  // 높을수록 동굴 적게
    float CaveStrength = 1200.0f; // "density 단위가 cm"라서 이건 cm급으로 줘야 체감됨
    float CaveBand = 0.25f;

    explicit FVoxelDensityGenerator(int32 InSeed) : Seed(InSeed) {}

    FORCEINLINE float SeedOffsetCm() const
    {
        // Seed
        return (float)Seed * 100000.0f;
    }

    float GetDensity(int32 X, int32 Y, int32 Z) const;
};