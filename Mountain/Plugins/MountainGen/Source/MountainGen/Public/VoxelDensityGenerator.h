#pragma once
#include "CoreMinimal.h"

// Density(x,y,z) = Height(x,y) - z + Cave(x,y,z)
// Density > 0 => Solid
struct FVoxelDensityGenerator
{
    int32 Seed = 1337;

    // 산의 큰 윤곽(2D)
    float HeightScale = 0.02f;     // 작을수록 덩어리 큼
    float HeightAmp = 20.0f;     // 최대 높이(복셀 단위 느낌)

    // 동굴/오버행(3D)
    float CaveScale = 0.06f;
    float CaveStrength = 2.0f;

    // 추가로 전체 고도 올리고 싶을 때
    float BaseFloor = 10.0f;       // 전체를 더 solid로 만들고 싶으면 올려라

    FVoxelDensityGenerator() = default;
    explicit FVoxelDensityGenerator(int32 InSeed) : Seed(InSeed) {}

    float GetDensity(int32 X, int32 Y, int32 Z) const;

private:
    FVector2D SeedOffset2D() const;
    FVector   SeedOffset3D() const;
};