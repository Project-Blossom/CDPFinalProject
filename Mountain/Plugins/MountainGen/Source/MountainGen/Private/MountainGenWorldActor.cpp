#include "MountainGenWorldActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"
#include "MountainGenMeshData.h"

AMountainGenWorldActor::AMountainGenWorldActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = false;
}

void AMountainGenWorldActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::Regenerate()
{
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::SetSeed(int32 NewSeed)
{
    Settings.Seed = NewSeed;
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;
    ProcMesh->ClearAllMeshSections();

    // Mesher는 샘플 (N+1)을 요구하므로 +1.
    const int32 SampleX = Settings.ChunkX + 1;
    const int32 SampleY = Settings.ChunkY + 1;
    const int32 SampleZ = Settings.ChunkZ + 1;

    FVoxelChunk Chunk;
    Chunk.Init(SampleX, SampleY, SampleZ);

    const float Voxel = Settings.VoxelSizeCm;

    const FVector TerrainOrigin = GetActorLocation();
    FVoxelDensityGenerator Gen(Settings, TerrainOrigin);

    for (int32 z = 0; z < SampleZ; ++z)
        for (int32 y = 0; y < SampleY; ++y)
            for (int32 x = 0; x < SampleX; ++x)
            {
                const FVector WorldPos = TerrainOrigin + FVector(x * Voxel, y * Voxel, z * Voxel);
                Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
            }

    FChunkMeshData MeshData;

    // 액터 기준 로컬로 만들려면 ChunkOriginWorld는 0으로 두고,
    // 샘플 좌표도 로컬이 되게 해야 하는데, 지금은 WorldPos로 샘플링했기 때문에
    // "정점 생성"은 로컬로 고정하는 게 깔끔하다.
    //
    // => ChunkOriginWorld = FVector::ZeroVector 로 주면 정점은 (0..size) 로컬 범위에 생성됨.
    FVoxelMesher::BuildMarchingCubes(
        Chunk,
        Settings.VoxelSizeCm,
        Settings.IsoLevel,
        FVector::ZeroVector,
        MeshData
    );

    if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        return;

    TArray<FLinearColor> Colors;
    Colors.SetNumZeroed(MeshData.Vertices.Num());

    ProcMesh->CreateMeshSection_LinearColor(
        0,
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.Normals,
        MeshData.UV0,
        Colors,
        MeshData.Tangents,
        Settings.bCreateCollision
    );

    ProcMesh->SetCollisionEnabled(
        Settings.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
    );

    if (VoxelMaterial)
        ProcMesh->SetMaterial(0, VoxelMaterial);
}
