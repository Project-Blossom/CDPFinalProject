#pragma once
#include "CoreMinimal.h"

// 3D Density Grid (Density > 0 : Solid, <= 0 : Empty)
struct FVoxelChunk
{
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 SizeZ = 0;

    // Density values
    TArray<float> Density;

    FVoxelChunk() = default;

    FVoxelChunk(int32 InX, int32 InY, int32 InZ)
    {
        Init(InX, InY, InZ);
    }

    void Init(int32 InX, int32 InY, int32 InZ)
    {
        SizeX = InX;
        SizeY = InY;
        SizeZ = InZ;
        Density.SetNum(SizeX * SizeY * SizeZ);
    }

    FORCEINLINE bool IsInside(int32 X, int32 Y, int32 Z) const
    {
        return (X >= 0 && X < SizeX &&
            Y >= 0 && Y < SizeY &&
            Z >= 0 && Z < SizeZ);
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