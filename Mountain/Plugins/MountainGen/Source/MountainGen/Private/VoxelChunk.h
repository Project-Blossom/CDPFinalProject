#pragma once
#include "CoreMinimal.h"

// Density < 0 : Solid, >= 0 : Air
struct FVoxelChunk
{
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 SizeZ = 0;

    TArray<float> Density;

    void Init(int32 InX, int32 InY, int32 InZ)
    {
        SizeX = InX; SizeY = InY; SizeZ = InZ;
        Density.SetNum(SizeX * SizeY * SizeZ);
    }

    FORCEINLINE int32 Index(int32 X, int32 Y, int32 Z) const
    {
        return X + Y * SizeX + Z * SizeX * SizeY;
    }

    FORCEINLINE float Get(int32 X, int32 Y, int32 Z) const
    {
        return Density[Index(X, Y, Z)];
    }

    FORCEINLINE void Set(int32 X, int32 Y, int32 Z, float Value)
    {
        Density[Index(X, Y, Z)] = Value;
    }
};