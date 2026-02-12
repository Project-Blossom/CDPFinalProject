#include "VoxelMesher.h"

#include "VoxelChunk.h"
#include "MountainGenMeshData.h"
#include "MarchingCubesTables.h"
#include "VoxelDensityGenerator.h"

#include "ProceduralMeshComponent.h"

namespace
{
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

    static FORCEINLINE FProcMeshTangent MakeTangentFromNormal(const FVector& N)
    {
        const FVector Ref = (FMath::Abs(N.Z) < 0.999f) ? FVector::UpVector : FVector::ForwardVector;
        FVector T = (Ref - (Ref | N) * N).GetSafeNormal();
        if (!T.IsNormalized()) T = FVector::RightVector;
        return FProcMeshTangent(T, false);
    }

    static FORCEINLINE float LerpT(float V1, float V2, float Iso)
    {
        const float Den = (V2 - V1);
        if (FMath::IsNearlyZero(Den)) return 0.0f;
        return (Iso - V1) / Den;
    }
}

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

    // ------------------------------------------------------------
    // Normals
    // ------------------------------------------------------------
    auto SampleClamped = [&](int32 x, int32 y, int32 z) -> float
        {
            x = FMath::Clamp(x, 0, SX - 1);
            y = FMath::Clamp(y, 0, SY - 1);
            z = FMath::Clamp(z, 0, SZ - 1);
            return Sample(x, y, z);
        };

    TArray<FVector> Grad;
    Grad.SetNumUninitialized(SX * SY * SZ);

    for (int32 gz = 0; gz < SZ; ++gz)
    {
        for (int32 gy = 0; gy < SY; ++gy)
        {
            for (int32 gx = 0; gx < SX; ++gx)
            {
                const float dx = SampleClamped(gx + 1, gy, gz) - SampleClamped(gx - 1, gy, gz);
                const float dy = SampleClamped(gx, gy + 1, gz) - SampleClamped(gx, gy - 1, gz);
                const float dz = SampleClamped(gx, gy, gz + 1) - SampleClamped(gx, gy, gz - 1);
                Grad[Idx3(gx, gy, gz, SX, SY)] = FVector(dx, dy, dz);
            }
        }
    }

    auto GradAt = [&](int32 x, int32 y, int32 z) -> FVector
        {
            x = FMath::Clamp(x, 0, SX - 1);
            y = FMath::Clamp(y, 0, SY - 1);
            z = FMath::Clamp(z, 0, SZ - 1);
            return Grad[Idx3(x, y, z, SX, SY)];
        };

    static const FIntVector CornerOfs[8] =
    {
        FIntVector(0, 0, 0),
        FIntVector(1, 0, 0),
        FIntVector(1, 1, 0),
        FIntVector(0, 1, 0),
        FIntVector(0, 0, 1),
        FIntVector(1, 0, 1),
        FIntVector(1, 1, 1),
        FIntVector(0, 1, 1),
    };


    auto WorldP = [&](int32 x, int32 y, int32 z) -> FVector
        {
            return ChunkOriginWorld + FVector(x * VoxelSizeCm, y * VoxelSizeCm, z * VoxelSizeCm);
        };

    const int32 XW = SX - 1;
    const int32 YW = SY - 1;
    const int32 ZW = SZ - 1;
    if (XW <= 0 || YW <= 0 || ZW <= 0) return;

    const int32 NumXEdges = XW * SY * SZ;
    const int32 NumYEdges = SX * YW * SZ;
    const int32 NumZEdges = SX * SY * ZW;

    // --- Reusable edge caches ---------------------------------
    static thread_local TArray<int32> XEdgeCache;
    static thread_local TArray<int32> YEdgeCache;
    static thread_local TArray<int32> ZEdgeCache;
    static thread_local int32 CachedNumXEdges = 0;
    static thread_local int32 CachedNumYEdges = 0;
    static thread_local int32 CachedNumZEdges = 0;

    auto EnsureEdgeCache = [](TArray<int32>& Cache, int32& CachedNum, int32 NeededNum)
        {
            if (CachedNum != NeededNum)
            {
                Cache.SetNumUninitialized(NeededNum);
                CachedNum = NeededNum;
            }
        };

    EnsureEdgeCache(XEdgeCache, CachedNumXEdges, NumXEdges);
    EnsureEdgeCache(YEdgeCache, CachedNumYEdges, NumYEdges);
    EnsureEdgeCache(ZEdgeCache, CachedNumZEdges, NumZEdges);

    FMemory::Memset(XEdgeCache.GetData(), 0xFF, sizeof(int32) * XEdgeCache.Num());
    FMemory::Memset(YEdgeCache.GetData(), 0xFF, sizeof(int32) * YEdgeCache.Num());
    FMemory::Memset(ZEdgeCache.GetData(), 0xFF, sizeof(int32) * ZEdgeCache.Num());

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

    auto AddSharedVertex = [&](const FVector& PWorld, const FVector& NIn) -> int32
        {
            const int32 Idx = Out.Vertices.Num();
            Out.Vertices.Add(PWorld - ActorWorld);

            FVector N = NIn;
            N *= -1.f;

            if (!N.IsNormalized()) N = FVector::UpVector;
            Out.Normals.Add(N);

            const FVector PLocal = (PWorld - ActorWorld);
            Out.UV0.Add(FVector2D(PLocal.X * 0.001f, PLocal.Y * 0.001f));

            Out.Tangents.Add(MakeTangentFromNormal(N));
            return Idx;
        };


    auto MakeAndCache = [&](int32 x, int32 y, int32 z, int32& CacheSlot, int32 cA, int32 cB, const float V[8], const FVector P[8]) -> int32
        {
            if (CacheSlot != -1) return CacheSlot;

            const FVector PV = LerpVertex(P[cA], P[cB], V[cA], V[cB], IsoLevel);

            const float t = FMath::Clamp(LerpT(V[cA], V[cB], IsoLevel), 0.0f, 1.0f);

            const FIntVector OA = CornerOfs[cA];
            const FIntVector OB = CornerOfs[cB];

            const FVector GA = GradAt(x + OA.X, y + OA.Y, z + OA.Z);
            const FVector GB = GradAt(x + OB.X, y + OB.Y, z + OB.Z);
            const FVector G = (GA + (GB - GA) * t);

            const FVector N = G.GetSafeNormal();
            CacheSlot = AddSharedVertex(PV, N);
            return CacheSlot;
        };


    auto GetEdgeVertexIndex = [&](int32 x, int32 y, int32 z, int32 e, const float V[8], const FVector P[8]) -> int32
        {
            switch (e)
            {
            case 0: { int32& slot = XEdgeCache[XEdgeIdx(x, y, z)];     return MakeAndCache(x, y, z, slot, 0, 1, V, P); }
            case 1: { int32& slot = YEdgeCache[YEdgeIdx(x + 1, y, z)];     return MakeAndCache(x, y, z, slot, 1, 2, V, P); }
            case 2: { int32& slot = XEdgeCache[XEdgeIdx(x, y + 1, z)];     return MakeAndCache(x, y, z, slot, 2, 3, V, P); }
            case 3: { int32& slot = YEdgeCache[YEdgeIdx(x, y, z)];     return MakeAndCache(x, y, z, slot, 3, 0, V, P); }

            case 4: { int32& slot = XEdgeCache[XEdgeIdx(x, y, z + 1)]; return MakeAndCache(x, y, z, slot, 4, 5, V, P); }
            case 5: { int32& slot = YEdgeCache[YEdgeIdx(x + 1, y, z + 1)]; return MakeAndCache(x, y, z, slot, 5, 6, V, P); }
            case 6: { int32& slot = XEdgeCache[XEdgeIdx(x, y + 1, z + 1)]; return MakeAndCache(x, y, z, slot, 6, 7, V, P); }
            case 7: { int32& slot = YEdgeCache[YEdgeIdx(x, y, z + 1)]; return MakeAndCache(x, y, z, slot, 7, 4, V, P); }

            case 8: { int32& slot = ZEdgeCache[ZEdgeIdx(x, y, z)];     return MakeAndCache(x, y, z, slot, 0, 4, V, P); }
            case 9: { int32& slot = ZEdgeCache[ZEdgeIdx(x + 1, y, z)];     return MakeAndCache(x, y, z, slot, 1, 5, V, P); }
            case 10: { int32& slot = ZEdgeCache[ZEdgeIdx(x + 1, y + 1, z)];     return MakeAndCache(x, y, z, slot, 2, 6, V, P); }
            case 11: { int32& slot = ZEdgeCache[ZEdgeIdx(x, y + 1, z)];     return MakeAndCache(x, y, z, slot, 3, 7, V, P); }
            default: return -1;
            }
        };

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