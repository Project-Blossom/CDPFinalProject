#pragma once
#include "CoreMinimal.h"

struct FVoxelChunk;
struct FChunkMeshData;
struct FVoxelDensityGenerator;

struct FVoxelMesher
{
    static void BuildMarchingCubes(
        const FVoxelChunk& Chunk,
        float VoxelSizeCm,
        float IsoLevel,
        const FVector& ChunkOriginWorld,
        const FVoxelDensityGenerator& Gen,
        FChunkMeshData& Out);
};