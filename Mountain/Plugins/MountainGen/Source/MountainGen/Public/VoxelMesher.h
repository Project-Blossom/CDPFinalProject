#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"   // 🔥 반드시 필요
#include "VoxelChunk.h"

// 메쉬 출력 버퍼
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
    static void BuildSurface(const FVoxelChunk& Chunk, float VoxelSize, FVoxelMeshData& Out);

    static void BuildHeightfieldSurface(const FVoxelChunk& Chunk, float VoxelSize, FVoxelMeshData& Out);

    static void BuildMarchingCubes(const FVoxelChunk& Chunk, float VoxelSize, float IsoLevel, FVoxelMeshData& Out);
};