#include "MountainGenVoxelActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "KismetProceduralMeshLibrary.h"

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

        Gen.WorldScaleCm = WorldScaleCm;
        Gen.DetailScaleCm = DetailScaleCm;
        Gen.CaveScaleCm = CaveScaleCm;
        Gen.BaseBias = BaseBias;
        Gen.OverhangAmp = OverhangAmp;
        Gen.CaveAmp = CaveAmp;
        Gen.CaveThreshold = CaveThreshold;
        Gen.VoxelSizeCm = VoxelSize;

        // -------------------------
        // 1) Density 필드 생성
        // -------------------------
        for (int32 z = 0; z < ChunkZ; ++z)
            for (int32 y = 0; y < ChunkY; ++y)
                for (int32 x = 0; x < ChunkX; ++x)
                    Chunk.Set(x, y, z, Gen.GetDensity(x, y, z));

        // -------------------------
        // 2) Marching Cubes
        // -------------------------
        FVoxelMeshData Mesh;
        FVoxelMesher::BuildMarchingCubes(Chunk, VoxelSize, 0.0f, Mesh);

        if (Mesh.UVs.Num() != Mesh.Vertices.Num())
        {
            Mesh.UVs.SetNum(Mesh.Vertices.Num());
            for (int32 i = 0; i < Mesh.Vertices.Num(); ++i)
            {
                const FVector& P = Mesh.Vertices[i];
                Mesh.UVs[i] = FVector2D(P.X * 0.001f, P.Y * 0.001f);
            }
        }

        // -------------------------
        // 3) 노멀/탄젠트 계산
        // -------------------------
        TArray<FVector> CalcNormals;
        TArray<FProcMeshTangent> CalcTangents;
        UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
            Mesh.Vertices,
            Mesh.Triangles,
            Mesh.UVs,
            CalcNormals,
            CalcTangents
        );

        ProcMesh->CreateMeshSection_LinearColor(
            0,
            Mesh.Vertices,
            Mesh.Triangles,
            CalcNormals,
            Mesh.UVs,
            Mesh.Colors,
            CalcTangents,
            true
        );

        ProcMesh->bUseComplexAsSimpleCollision = true;

        if (VoxelMaterial)
            ProcMesh->SetMaterial(0, VoxelMaterial);
    }