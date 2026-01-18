#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "VoxelChunk.h"

struct FVoxelMeshData
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    void Reset()
    {
        Vertices.Reset();
        Triangles.Reset();
        Normals.Reset();
        UVs.Reset();
        Colors.Reset();
        Tangents.Reset();
    }
};

struct FVoxelMesher
{
    static void BuildMarchingCubes(const FVoxelChunk& Chunk, float VoxelSize, float IsoLevel, FVoxelMeshData& Out);
};