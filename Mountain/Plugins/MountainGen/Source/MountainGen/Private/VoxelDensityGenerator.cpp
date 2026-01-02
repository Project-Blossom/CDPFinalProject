#include "VoxelDensityGenerator.h"

static FORCEINLINE float Noise3D(float x, float y, float z)
{
    return FMath::PerlinNoise3D(FVector(x, y, z)); // -1 ~ 1
}

float FVoxelDensityGenerator::GetDensity(int32 X, int32 Y, int32 Z) const
{
    const float wx = X * 100.0f;
    const float wy = Y * 100.0f;
    const float wz = Z * 100.0f;

    float base = (wz - 3000.0f);  // 높아질수록 +

    // 노이즈
    float n1 = FMath::PerlinNoise3D(FVector(wx, wy, wz) * 0.002f);
    float n2 = FMath::PerlinNoise3D(FVector(wx, wy, wz) * 0.01f);

    // 합치기
    float density = base + (n1 * 400.f) + (n2 * 120.f);

    // 🔥 핵심 수정 (부호 반전)
    return -density;
}