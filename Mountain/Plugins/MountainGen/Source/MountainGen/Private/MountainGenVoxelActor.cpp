#include "MountainGenVoxelActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"

AMountainGenVoxelActor::AMountainGenVoxelActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = false; // 디버그/안정 우선
}

void AMountainGenVoxelActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    BuildChunkAndMesh();
}

void AMountainGenVoxelActor::Regenerate()
{
    BuildChunkAndMesh();
}

void AMountainGenVoxelActor::SetSeed(int32 NewSeed)
{
    Seed = NewSeed;
    BuildChunkAndMesh();
}

void AMountainGenVoxelActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    ProcMesh->ClearAllMeshSections();

    FVoxelChunk Chunk(ChunkX, ChunkY, ChunkZ);

    FVoxelDensityGenerator Gen(Seed);
    Gen.HeightScale = HeightScale;
    Gen.HeightAmp = HeightAmp;
    Gen.CaveScale = CaveScale;
    Gen.CaveStrength = CaveStrength;
    Gen.BaseFloor = BaseFloor;

    for (int32 z = 0; z < ChunkZ; ++z)
        for (int32 y = 0; y < ChunkY; ++y)
            for (int32 x = 0; x < ChunkX; ++x)
            {
                const float D = Gen.GetDensity(x, y, z);
                Chunk.Set(x, y, z, D);
            }

    FVoxelMeshData Mesh;
    FVoxelMesher::BuildMarchingCubes(Chunk, VoxelSize, 0.0f, Mesh);

    // ✅ UE가 노멀/탄젠트를 만들도록 빈 배열 전달
    TArray<FVector> EmptyNormals;
    TArray<FProcMeshTangent> EmptyTangents;

    ProcMesh->CreateMeshSection_LinearColor(
        0,
        Mesh.Vertices,
        Mesh.Triangles,
        EmptyNormals,          // ✅ 비움
        Mesh.UVs,
        Mesh.Colors,
        EmptyTangents,         // ✅ 비움
        true                   // collision
    );

    // 충돌
    ProcMesh->bUseComplexAsSimpleCollision = true;

    if (VoxelMaterial)
    {
        ProcMesh->SetMaterial(0, VoxelMaterial);
    }
}