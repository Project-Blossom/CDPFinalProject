#pragma once
#include "CoreMinimal.h"
#include "MountainGenMeshData.h"

struct FVoxelChunk;

class FVoxelMesher
{
public:
    static void BuildMarchingCubes(
        const FVoxelChunk& Chunk,
        float VoxelSizeCm,
        float IsoLevel,
        const FVector& ChunkOriginWorld,
        FChunkMeshData& Out
    );
};