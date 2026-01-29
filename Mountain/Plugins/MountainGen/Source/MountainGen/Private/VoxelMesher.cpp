#include "VoxelMesher.h"

#include "VoxelChunk.h"
#include "MountainGenMeshData.h"
#include "MarchingCubesTables.h"
#include "VoxelDensityGenerator.h"

#include "ProceduralMeshComponent.h"

namespace
{
    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------
    static FORCEINLINE int32 Idx3(int32 X, int32 Y, int32 Z, int32 SX, int32 SY)
    {
        return X + Y * SX + Z * SX * SY;
    }

    static FORCEINLINE FVector LerpVertex(const FVector& P1, const FVector& P2, float V1, float V2, float Iso)
    {
        const float Den = (V2 - V1);
        if (FMath::IsNearlyZero(Den)) return P1;

        const float T = (Iso - V1) / Den;
        return P1 + T * (P2 - P1);
    }

    static FORCEINLINE FVector EstimateNormalFromDensity(const FVoxelDensityGenerator& Gen, const FVector& Pcm, float StepCm)
    {
        const float dx = Gen.SampleDensity(Pcm + FVector(StepCm, 0, 0)) - Gen.SampleDensity(Pcm - FVector(StepCm, 0, 0));
        const float dy = Gen.SampleDensity(Pcm + FVector(0, StepCm, 0)) - Gen.SampleDensity(Pcm - FVector(0, StepCm, 0));
        const float dz = Gen.SampleDensity(Pcm + FVector(0, 0, StepCm)) - Gen.SampleDensity(Pcm - FVector(0, 0, StepCm));
        return FVector(dx, dy, dz).GetSafeNormal();
    }

    static FORCEINLINE FProcMeshTangent MakeTangentFromNormal(const FVector& N)
    {
        const FVector Ref = (FMath::Abs(N.Z) < 0.999f) ? FVector::UpVector : FVector::ForwardVector;
        FVector T = (Ref - (Ref | N) * N).GetSafeNormal();
        if (!T.IsNormalized()) T = FVector::RightVector;
        return FProcMeshTangent(T, false);
    }
} // namespace

// ------------------------------------------------------------
// FVoxelMesher
// ------------------------------------------------------------
void FVoxelMesher::BuildMarchingCubes(
    const FVoxelChunk& Chunk,
    float VoxelSizeCm,
    float IsoLevel,
    const FVector& ChunkOriginWorld,
    const FVector& ActorWorld,
    const FVoxelDensityGenerator& Gen,
    FChunkMeshData& Out)
{
    Out.Vertices.Reset();
    Out.Triangles.Reset();
    Out.Normals.Reset();
    Out.UV0.Reset();
    Out.Tangents.Reset();

    const int32 SX = Chunk.SizeX;
    const int32 SY = Chunk.SizeY;
    const int32 SZ = Chunk.SizeZ;

    const int32 CX = SX - 1;
    const int32 CY = SY - 1;
    const int32 CZ = SZ - 1;
    if (CX <= 0 || CY <= 0 || CZ <= 0) return;

    auto Sample = [&](int32 x, int32 y, int32 z) -> float
        {
            return Chunk.Density[Idx3(x, y, z, SX, SY)];
        };

    auto WorldP = [&](int32 x, int32 y, int32 z) -> FVector
        {
            return ChunkOriginWorld + FVector(x * VoxelSizeCm, y * VoxelSizeCm, z * VoxelSizeCm);
        };

    // ============================================================
    // Edge caches
    // ============================================================
    const int32 XW = SX - 1;
    const int32 YW = SY - 1;
    const int32 ZW = SZ - 1;

    if (XW <= 0 || YW <= 0 || ZW <= 0) return;

    const int32 NumXEdges = XW * SY * SZ;
    const int32 NumYEdges = SX * YW * SZ;
    const int32 NumZEdges = SX * SY * ZW;

    TArray<int32> XEdgeCache; XEdgeCache.Init(-1, NumXEdges);
    TArray<int32> YEdgeCache; YEdgeCache.Init(-1, NumYEdges);
    TArray<int32> ZEdgeCache; ZEdgeCache.Init(-1, NumZEdges);

    auto XEdgeIdx = [&](int32 x, int32 y, int32 z) -> int32
        {
            return x + y * XW + z * XW * SY;
        };
    auto YEdgeIdx = [&](int32 x, int32 y, int32 z) -> int32
        {
            return x + y * SX + z * SX * YW;
        };
    auto ZEdgeIdx = [&](int32 x, int32 y, int32 z) -> int32
        {
            return x + y * SX + z * SX * SY;
        };

    auto AddSharedVertex = [&](const FVector& PWorld) -> int32
        {
            const int32 Idx = Out.Vertices.Num();

            Out.Vertices.Add(PWorld - ActorWorld);

            const float Step = FMath::Max(2.0f * VoxelSizeCm, 40.0f);

            FVector N = -EstimateNormalFromDensity(Gen, PWorld, Step);
            if (!N.IsNormalized()) N = FVector::UpVector;

            Out.Normals.Add(N);

            const FVector PLocal = (PWorld - ActorWorld);
            Out.UV0.Add(FVector2D(PLocal.X * 0.001f, PLocal.Y * 0.001f));

            Out.Tangents.Add(MakeTangentFromNormal(N));
            return Idx;
        };

    auto MakeAndCache = [&](int32& CacheSlot, int32 cA, int32 cB, const float V[8], const FVector P[8]) -> int32
        {
            if (CacheSlot != -1) return CacheSlot;

            const FVector PV = LerpVertex(P[cA], P[cB], V[cA], V[cB], IsoLevel);
            CacheSlot = AddSharedVertex(PV);
            return CacheSlot;
        };

    auto GetEdgeVertexIndex = [&](int32 x, int32 y, int32 z, int32 e, const float V[8], const FVector P[8]) -> int32
        {
            switch (e)
            {
            case 0: { int32& slot = XEdgeCache[XEdgeIdx(x, y, z)];     return MakeAndCache(slot, 0, 1, V, P); }
            case 1: { int32& slot = YEdgeCache[YEdgeIdx(x + 1, y, z)];     return MakeAndCache(slot, 1, 2, V, P); }
            case 2: { int32& slot = XEdgeCache[XEdgeIdx(x, y + 1, z)];     return MakeAndCache(slot, 2, 3, V, P); }
            case 3: { int32& slot = YEdgeCache[YEdgeIdx(x, y, z)];     return MakeAndCache(slot, 3, 0, V, P); }

            case 4: { int32& slot = XEdgeCache[XEdgeIdx(x, y, z + 1)]; return MakeAndCache(slot, 4, 5, V, P); }
            case 5: { int32& slot = YEdgeCache[YEdgeIdx(x + 1, y, z + 1)]; return MakeAndCache(slot, 5, 6, V, P); }
            case 6: { int32& slot = XEdgeCache[XEdgeIdx(x, y + 1, z + 1)]; return MakeAndCache(slot, 6, 7, V, P); }
            case 7: { int32& slot = YEdgeCache[YEdgeIdx(x, y, z + 1)]; return MakeAndCache(slot, 7, 4, V, P); }

            case 8: { int32& slot = ZEdgeCache[ZEdgeIdx(x, y, z)];     return MakeAndCache(slot, 0, 4, V, P); }
            case 9: { int32& slot = ZEdgeCache[ZEdgeIdx(x + 1, y, z)];     return MakeAndCache(slot, 1, 5, V, P); }
            case 10: { int32& slot = ZEdgeCache[ZEdgeIdx(x + 1, y + 1, z)];     return MakeAndCache(slot, 2, 6, V, P); }
            case 11: { int32& slot = ZEdgeCache[ZEdgeIdx(x, y + 1, z)];     return MakeAndCache(slot, 3, 7, V, P); }
            default: return -1;
            }
        };

    // ============================================================
    // Triangulation
    // ============================================================
    Out.Vertices.Reserve(NumXEdges + NumYEdges + NumZEdges);
    Out.Normals.Reserve(NumXEdges + NumYEdges + NumZEdges);
    Out.UV0.Reserve(NumXEdges + NumYEdges + NumZEdges);
    Out.Tangents.Reserve(NumXEdges + NumYEdges + NumZEdges);
    Out.Triangles.Reserve(CX * CY * CZ * 15);

    for (int32 z = 0; z < CZ; ++z)
    {
        for (int32 y = 0; y < CY; ++y)
        {
            for (int32 x = 0; x < CX; ++x)
            {
                float V[8];
                V[0] = Sample(x, y, z);
                V[1] = Sample(x + 1, y, z);
                V[2] = Sample(x + 1, y + 1, z);
                V[3] = Sample(x, y + 1, z);
                V[4] = Sample(x, y, z + 1);
                V[5] = Sample(x + 1, y, z + 1);
                V[6] = Sample(x + 1, y + 1, z + 1);
                V[7] = Sample(x, y + 1, z + 1);

                int32 CubeIndex = 0;
                if (V[0] < IsoLevel) CubeIndex |= 1;
                if (V[1] < IsoLevel) CubeIndex |= 2;
                if (V[2] < IsoLevel) CubeIndex |= 4;
                if (V[3] < IsoLevel) CubeIndex |= 8;
                if (V[4] < IsoLevel) CubeIndex |= 16;
                if (V[5] < IsoLevel) CubeIndex |= 32;
                if (V[6] < IsoLevel) CubeIndex |= 64;
                if (V[7] < IsoLevel) CubeIndex |= 128;

                const int32 Edges = EdgeTable[CubeIndex];
                if (Edges == 0) continue;

                FVector P[8];
                P[0] = WorldP(x, y, z);
                P[1] = WorldP(x + 1, y, z);
                P[2] = WorldP(x + 1, y + 1, z);
                P[3] = WorldP(x, y + 1, z);
                P[4] = WorldP(x, y, z + 1);
                P[5] = WorldP(x + 1, y, z + 1);
                P[6] = WorldP(x + 1, y + 1, z + 1);
                P[7] = WorldP(x, y + 1, z + 1);

                // 삼각형 생성
                for (int32 i = 0; TriTable[CubeIndex][i] != -1; i += 3)
                {
                    const int32 eA = TriTable[CubeIndex][i];
                    const int32 eB = TriTable[CubeIndex][i + 1];
                    const int32 eC = TriTable[CubeIndex][i + 2];

                    const int32 iA = GetEdgeVertexIndex(x, y, z, eA, V, P);
                    const int32 iB = GetEdgeVertexIndex(x, y, z, eB, V, P);
                    const int32 iC = GetEdgeVertexIndex(x, y, z, eC, V, P);

                    if (iA < 0 || iB < 0 || iC < 0) continue;
                    if (iA == iB || iB == iC || iC == iA) continue;

                    Out.Triangles.Add(iA);
                    Out.Triangles.Add(iC);
                    Out.Triangles.Add(iB);
                }
            }
        }
    }
}