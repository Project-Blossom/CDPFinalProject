#include "MountainGenVoxelActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"

static FORCEINLINE int32 Idx3(int32 X, int32 Y, int32 Z, int32 SX, int32 SY)
{
    return X + Y * SX + Z * SX * SY;
}

static void MakeSolidMask(const FVoxelChunk& Chunk, float IsoLevel, TArray<uint8>& OutSolid)
{
    OutSolid.SetNumZeroed(Chunk.SizeX * Chunk.SizeY * Chunk.SizeZ);

    for (int32 z = 0; z < Chunk.SizeZ; ++z)
        for (int32 y = 0; y < Chunk.SizeY; ++y)
            for (int32 x = 0; x < Chunk.SizeX; ++x)
            {
                const float d = Chunk.Get(x, y, z);
                OutSolid[Idx3(x, y, z, Chunk.SizeX, Chunk.SizeY)] = (d < IsoLevel) ? 1 : 0;
            }
}

// 6-neighborhood
static FORCEINLINE void Get6(int32 x, int32 y, int32 z, int32(&nx)[6], int32(&ny)[6], int32(&nz)[6])
{
    nx[0] = x - 1; ny[0] = y;   nz[0] = z;
    nx[1] = x + 1; ny[1] = y;   nz[1] = z;
    nx[2] = x;   ny[2] = y - 1; nz[2] = z;
    nx[3] = x;   ny[3] = y + 1; nz[3] = z;
    nx[4] = x;   ny[4] = y;   nz[4] = z - 1;
    nx[5] = x;   ny[5] = y;   nz[5] = z + 1;
}

static void DilateOnce(const FVoxelChunk& Chunk, const TArray<uint8>& In, TArray<uint8>& Out)
{
    Out = In;

    const int32 SX = Chunk.SizeX, SY = Chunk.SizeY, SZ = Chunk.SizeZ;
    int32 nx[6], ny[6], nz[6];

    for (int32 z = 0; z < SZ; ++z)
        for (int32 y = 0; y < SY; ++y)
            for (int32 x = 0; x < SX; ++x)
            {
                const int32 i = Idx3(x, y, z, SX, SY);
                if (In[i]) continue;

                Get6(x, y, z, nx, ny, nz);

                bool bNeighborSolid = false;
                for (int k = 0; k < 6; ++k)
                {
                    const int32 X2 = nx[k], Y2 = ny[k], Z2 = nz[k];
                    if (X2 < 0 || X2 >= SX || Y2 < 0 || Y2 >= SY || Z2 < 0 || Z2 >= SZ) continue;
                    if (In[Idx3(X2, Y2, Z2, SX, SY)]) { bNeighborSolid = true; break; }
                }

                if (bNeighborSolid) Out[i] = 1;
            }
}

static void ErodeOnce(const FVoxelChunk& Chunk, const TArray<uint8>& In, TArray<uint8>& Out)
{
    Out = In;

    const int32 SX = Chunk.SizeX, SY = Chunk.SizeY, SZ = Chunk.SizeZ;
    int32 nx[6], ny[6], nz[6];

    for (int32 z = 0; z < SZ; ++z)
        for (int32 y = 0; y < SY; ++y)
            for (int32 x = 0; x < SX; ++x)
            {
                const int32 i = Idx3(x, y, z, SX, SY);
                if (!In[i]) continue;

                Get6(x, y, z, nx, ny, nz);

                bool bNeighborAir = false;
                for (int k = 0; k < 6; ++k)
                {
                    const int32 X2 = nx[k], Y2 = ny[k], Z2 = nz[k];
                    if (X2 < 0 || X2 >= SX || Y2 < 0 || Y2 >= SY || Z2 < 0 || Z2 >= SZ) continue;
                    if (!In[Idx3(X2, Y2, Z2, SX, SY)]) { bNeighborAir = true; break; }
                }

                if (bNeighborAir) Out[i] = 0;
            }
}

static void ApplyClosingMask(const FVoxelChunk& Chunk, TArray<uint8>& SolidMask, int32 DilateIters, int32 ErodeIters)
{
    if (DilateIters <= 0 && ErodeIters <= 0) return;

    TArray<uint8> A = SolidMask;
    TArray<uint8> B;

    for (int i = 0; i < DilateIters; ++i)
    {
        DilateOnce(Chunk, A, B);
        A = MoveTemp(B);
    }

    for (int i = 0; i < ErodeIters; ++i)
    {
        ErodeOnce(Chunk, A, B);
        A = MoveTemp(B);
    }

    SolidMask = MoveTemp(A);
}

// 하늘섬 제거
static void RemoveFloatingIslandsKeepGrounded(const FVoxelChunk& Chunk, TArray<uint8>& SolidMask, int32 GroundBandZ)
{
    const int32 SX = Chunk.SizeX, SY = Chunk.SizeY, SZ = Chunk.SizeZ;
    GroundBandZ = FMath::Clamp(GroundBandZ, 1, SZ);

    TArray<uint8> Visited;
    Visited.SetNumZeroed(SX * SY * SZ);

    TArray<int32> Queue;
    Queue.Reserve(SX * SY);

    auto Push = [&](int32 x, int32 y, int32 z)
        {
            const int32 idx = Idx3(x, y, z, SX, SY);
            if (!SolidMask[idx]) return;
            if (Visited[idx]) return;
            Visited[idx] = 1;
            Queue.Add(idx);
        };

    // 시작점: 바닥 band의 solid
    for (int32 z = 0; z < GroundBandZ; ++z)
        for (int32 y = 0; y < SY; ++y)
            for (int32 x = 0; x < SX; ++x)
                Push(x, y, z);

    // BFS
    int32 nx[6], ny[6], nz[6];

    for (int32 qi = 0; qi < Queue.Num(); ++qi)
    {
        const int32 idx = Queue[qi];
        const int32 z = idx / (SX * SY);
        const int32 rem = idx - z * (SX * SY);
        const int32 y = rem / SX;
        const int32 x = rem - y * SX;

        Get6(x, y, z, nx, ny, nz);

        for (int k = 0; k < 6; ++k)
        {
            const int32 X2 = nx[k], Y2 = ny[k], Z2 = nz[k];
            if (X2 < 0 || X2 >= SX || Y2 < 0 || Y2 >= SY || Z2 < 0 || Z2 >= SZ) continue;
            const int32 nidx = Idx3(X2, Y2, Z2, SX, SY);
            if (!SolidMask[nidx]) continue;
            if (Visited[nidx]) continue;
            Visited[nidx] = 1;
            Queue.Add(nidx);
        }
    }

    // 방문되지 않은 solid = 섬 → air로
    for (int32 i = 0; i < SolidMask.Num(); ++i)
        if (SolidMask[i] && !Visited[i])
            SolidMask[i] = 0;
}

static void ApplyMaskSoftToDensity(FVoxelChunk& Chunk, const TArray<uint8>& SolidMask, float IsoLevel, float PushCm)
{
    if (PushCm <= 0.0f) return;

    const int32 SX = Chunk.SizeX, SY = Chunk.SizeY, SZ = Chunk.SizeZ;

    for (int32 z = 0; z < SZ; ++z)
        for (int32 y = 0; y < SY; ++y)
            for (int32 x = 0; x < SX; ++x)
            {
                const int32 i = Idx3(x, y, z, SX, SY);
                const bool bSolid = (SolidMask[i] != 0);

                float d = Chunk.Get(x, y, z);

                if (bSolid)
                {
                    // solid인데 density가 air(>=Iso)로 나가려 하면 Iso 아래로 살짝 밀어줌
                    if (d >= IsoLevel)
                        d = IsoLevel - PushCm;
                }
                else
                {
                    // air인데 density가 solid(<Iso)로 들어오려 하면 Iso 위로 살짝 밀어줌
                    if (d < IsoLevel)
                        d = IsoLevel + PushCm;
                }

                Chunk.Set(x, y, z, d);
            }
}

// ============================================================
// Actor
// ============================================================

AMountainGenVoxelActor::AMountainGenVoxelActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = false;
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

    // -------------------------
    // 1) Density 생성
    // -------------------------
    FVoxelDensityGenerator Gen(Seed);
    Gen.VoxelSizeCm = VoxelSize;

    Gen.WorldScaleCm = WorldScaleCm;
    Gen.DetailScaleCm = DetailScaleCm;

    Gen.BaseHeightCm = BaseHeightCm;
    Gen.HeightAmpCm = HeightAmpCm;
    Gen.RampHeightCm = RampHeightCm;
    Gen.RampLengthCm = RampLengthCm;

    Gen.VolumeStrength = VolumeStrength;
    Gen.OverhangFadeCm = OverhangFadeCm;

    Gen.WarpPatchCm = WarpPatchCm;
    Gen.WarpAmpCm = WarpAmpCm;
    Gen.WarpStrength = WarpStrength;

    Gen.CaveScaleCm = CaveScaleCm;
    Gen.CaveThreshold = CaveThreshold;
    Gen.CaveStrength = CaveStrength;
    Gen.CaveBand = CaveBand;

    for (int32 z = 0; z < ChunkZ; ++z)
        for (int32 y = 0; y < ChunkY; ++y)
            for (int32 x = 0; x < ChunkX; ++x)
                Chunk.Set(x, y, z, Gen.GetDensity(x, y, z));

    // -------------------------
    // 2) Postprocess:
    // -------------------------
    if (bRemoveIslands || bUseClosing)
    {
        TArray<uint8> SolidMask;
        MakeSolidMask(Chunk, IsoLevel, SolidMask);

        if (bRemoveIslands)
            RemoveFloatingIslandsKeepGrounded(Chunk, SolidMask, GroundBandZ);

        if (bUseClosing)
            ApplyClosingMask(Chunk, SolidMask, ClosingDilateIters, ClosingErodeIters);

        ApplyMaskSoftToDensity(Chunk, SolidMask, IsoLevel, SoftPushCm);
    }

    // -------------------------
    // 3) Marching Cubes
    // -------------------------
    FVoxelMeshData MeshData;
    FVoxelMesher::BuildMarchingCubes(Chunk, VoxelSize, IsoLevel, MeshData);

    if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        return;

    // UV 보정
    if (MeshData.UVs.Num() != MeshData.Vertices.Num())
    {
        MeshData.UVs.SetNum(MeshData.Vertices.Num());
        for (int32 i = 0; i < MeshData.Vertices.Num(); ++i)
        {
            const FVector& P = MeshData.Vertices[i];
            MeshData.UVs[i] = FVector2D(P.X * 0.001f, P.Y * 0.001f);
        }
    }

    ProcMesh->CreateMeshSection_LinearColor(
        0,
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.Normals,
        MeshData.UVs,
        MeshData.Colors,
        MeshData.Tangents,
        true
    );

    ProcMesh->bUseComplexAsSimpleCollision = true;

    if (VoxelMaterial)
        ProcMesh->SetMaterial(0, VoxelMaterial);
}