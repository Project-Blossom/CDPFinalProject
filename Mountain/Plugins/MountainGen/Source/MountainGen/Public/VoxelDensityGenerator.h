#pragma once
#include "CoreMinimal.h"

struct FVoxelDensityGenerator
{
    int32 Seed = 1557;

    // 노이즈 주파수(격자 좌표 기준)
    float WorldFreq = 0.02f;
    float DetailFreq = 0.06f;
    float CaveFreq = 0.08f;

    // 강도(Amplitude)
    float HeightAmp = 3000.0f;
    float OverhangAmp = 0.6f;
    float CaveAmp = 1.0f;

    // 동굴 생성 기준(임계값)
    float CaveThreshold = 0.2f;

    // “위로 갈수록 공기”가 되게 만드는 경사/바이어스
    float GroundSlope = 0.05f;
    float BaseBias = 10.0f;

    explicit FVoxelDensityGenerator(int32 InSeed) : Seed(InSeed) {}

    float GetDensity(int32 X, int32 Y, int32 Z) const;
};