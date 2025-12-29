#include "VoxelDensityGenerator.h"

FVector2D FVoxelDensityGenerator::SeedOffset2D() const
{
    FRandomStream R(Seed);
    return FVector2D(R.FRandRange(-100000.f, 100000.f), R.FRandRange(-100000.f, 100000.f));
}

FVector FVoxelDensityGenerator::SeedOffset3D() const
{
    FRandomStream R(Seed * 9176 + 11);
    return FVector(R.FRandRange(-100000.f, 100000.f),
        R.FRandRange(-100000.f, 100000.f),
        R.FRandRange(-100000.f, 100000.f));
}

float FVoxelDensityGenerator::GetDensity(int32 X, int32 Y, int32 Z) const
{
    const FVector2D Off2D = SeedOffset2D();
    const FVector   Off3D = SeedOffset3D();

    // 2D Height (»ê¸ÆÀÇ Å« Èå¸§)
    const float H = FMath::PerlinNoise2D((FVector2D((float)X, (float)Y) + Off2D) * HeightScale) * HeightAmp;

    // 3D Cave (µ¿±¼/¿À¹öÇà)
    const float C = FMath::PerlinNoise3D((FVector((float)X, (float)Y, (float)Z) + Off3D) * CaveScale) * CaveStrength;

    // Density: ³ôÀÌ - z + µ¿±¼ + ¹Ù´Ú º¸Á¤
    return (H + BaseFloor) - (float)Z + C;
}