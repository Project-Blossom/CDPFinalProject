#include "VoxelMesher.h"

#include "VoxelChunk.h"
#include "MountainGenMeshData.h"
#include "MarchingCubesTables.h"
#include "VoxelDensityGenerator.h"

#include "ProceduralMeshComponent.h"
#include "Async/ParallelFor.h"

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
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

    struct FQuantKey
    {
        int32 X = 0;
        int32 Y = 0;
        int32 Z = 0;

        bool operator==(const FQuantKey& O) const
        {
            return X == O.X && Y == O.Y && Z == O.Z;
        }
    };

    FORCEINLINE uint32 GetTypeHash(const FQuantKey& K)
    {
        uint32 H = ::GetTypeHash(K.X);
        H = HashCombine(H, ::GetTypeHash(K.Y));
        H = HashCombine(H, ::GetTypeHash(K.Z));
        return H;
    }

    FORCEINLINE FQuantKey QuantizePos01mm(const FVector& PLocalCm)
    {
        constexpr float Q = 100.0f;
        FQuantKey K;
        K.X = FMath::RoundToInt(PLocalCm.X * Q);
        K.Y = FMath::RoundToInt(PLocalCm.Y * Q);
        K.Z = FMath::RoundToInt(PLocalCm.Z * Q);
        return K;
    }

    struct FLocalMesh
    {
        TArray<FVector>           Vertices;
        TArray<int32>             Triangles;
        TArray<FVector>           Normals;
        TArray<FVector2D>         UV0;
        TArray<FProcMeshTangent>  Tangents;
    };

    struct FStampedEdgeCache
    {
        TArray<int32> Index;
        TArray<int32> Stamp;
        int32         Num = 0;

        void Ensure(int32 NeededNum)
        {
            if (Num < NeededNum)
            {
                const int32 OldNum = Num;

                Index.SetNumUninitialized(NeededNum);
                Stamp.SetNumUninitialized(NeededNum);
                Num = NeededNum;

                FMemory::Memset(Stamp.GetData() + OldNum, 0, sizeof(int32) * (Num - OldNum));
            }
        }

        FORCEINLINE int32& SlotIndex(int32 i) { return Index[i]; }
        FORCEINLINE int32& SlotStamp(int32 i) { return Stamp[i]; }

        FORCEINLINE void TouchSlot(int32 i, int32 BuildStampValue)
        {
            int32& S = SlotStamp(i);
            if (S != BuildStampValue)
            {
                S = BuildStampValue;
                SlotIndex(i) = -1;
            }
        }
    };

    static thread_local int32 GBuildStampTL = 1;
    FORCEINLINE int32 NextBuildStampTL()
    {
        ++GBuildStampTL;
        if (GBuildStampTL == 0) GBuildStampTL = 1;
        return GBuildStampTL;
    }
}

void FVoxelMesher::BuildMarchingCubes(
    const FVoxelChunk& Chunk,
    float VoxelSizeCm,
    float IsoLevel,
    const FVector& ChunkOriginWorld,
    const FVector& ActorWorld,
    const FVoxelDensityGenerator& ,
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

    const int32 NumWorkers = FMath::Clamp(FPlatformMisc::NumberOfCoresIncludingHyperthreads(), 1, 64);
    const int32 SlabCount = FMath::Clamp(NumWorkers, 1, CZ);
    const int32 SlabZ = FMath::Max(1, (CZ + SlabCount - 1) / SlabCount);

    TArray<FLocalMesh> SlabMeshes;
    SlabMeshes.SetNum(SlabCount);

    ParallelFor(SlabCount, [&](int32 SlabIdx)
        {
            const int32 Z0 = SlabIdx * SlabZ;
            const int32 Z1 = FMath::Min(CZ, Z0 + SlabZ);

            FLocalMesh& LM = SlabMeshes[SlabIdx];
            LM.Vertices.Reset();
            LM.Triangles.Reset();
            LM.Normals.Reset();
            LM.UV0.Reset();
            LM.Tangents.Reset();

            if (Z0 >= Z1) return;

            const int32 XW = SX - 1;
            const int32 YW = SY - 1;

            const int32 ZPlaneCount = (Z1 - Z0) + 1;
            const int32 ZEdgeLayerCount = (Z1 - Z0);

            if (XW <= 0 || YW <= 0 || ZPlaneCount <= 1 || ZEdgeLayerCount <= 0) return;

            const int32 NumXEdges = XW * SY * ZPlaneCount;
            const int32 NumYEdges = SX * YW * ZPlaneCount;
            const int32 NumZEdges = SX * SY * ZEdgeLayerCount;

            static thread_local FStampedEdgeCache XCacheTL;
            static thread_local FStampedEdgeCache YCacheTL;
            static thread_local FStampedEdgeCache ZCacheTL;

            XCacheTL.Ensure(NumXEdges);
            YCacheTL.Ensure(NumYEdges);
            ZCacheTL.Ensure(NumZEdges);

            const int32 BuildStamp = NextBuildStampTL();

            auto XEdgeIdx = [&](int32 x, int32 y, int32 zPlaneLocal) -> int32
                {
                    return x + y * XW + zPlaneLocal * XW * SY;
                };
            auto YEdgeIdx = [&](int32 x, int32 y, int32 zPlaneLocal) -> int32
                {
                    return x + y * SX + zPlaneLocal * SX * YW;
                };
            auto ZEdgeIdx = [&](int32 x, int32 y, int32 zLayerLocal) -> int32
                {
                    return x + y * SX + zLayerLocal * SX * SY;
                };

            auto AddSharedVertex = [&](const FVector& PWorld) -> int32
                {
                    const int32 Idx = LM.Vertices.Num();
                    const FVector PLocal = (PWorld - ActorWorld);
                    LM.Vertices.Add(PLocal);

                    LM.Normals.Add(FVector::ZeroVector);
                    LM.UV0.Add(FVector2D(PLocal.X * 0.001f, PLocal.Y * 0.001f));
                    LM.Tangents.Add(FProcMeshTangent());
                    return Idx;
                };

            auto MakeAndCache = [&](FStampedEdgeCache& Cache, int32 Slot, int32 cA, int32 cB, const float V[8], const FVector P[8]) -> int32
                {
                    Cache.TouchSlot(Slot, BuildStamp);
                    int32& CachedIdx = Cache.SlotIndex(Slot);
                    if (CachedIdx != -1) return CachedIdx;

                    const FVector PV = LerpVertex(P[cA], P[cB], V[cA], V[cB], IsoLevel);
                    CachedIdx = AddSharedVertex(PV);
                    return CachedIdx;
                };

            auto GetEdgeVertexIndex = [&](int32 x, int32 y, int32 zCell, int32 e, const float V[8], const FVector P[8]) -> int32
                {
                    const int32 zL = zCell - Z0;

                    switch (e)
                    {
                        // X edges
                    case 0:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y, zL), 0, 1, V, P);
                    case 2:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y + 1, zL), 2, 3, V, P);
                    case 4:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y, zL + 1), 4, 5, V, P);
                    case 6:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y + 1, zL + 1), 6, 7, V, P);

                        // Y edges
                    case 1:  return MakeAndCache(YCacheTL, YEdgeIdx(x + 1, y, zL), 1, 2, V, P);
                    case 3:  return MakeAndCache(YCacheTL, YEdgeIdx(x, y, zL), 3, 0, V, P);
                    case 5:  return MakeAndCache(YCacheTL, YEdgeIdx(x + 1, y, zL + 1), 5, 6, V, P);
                    case 7:  return MakeAndCache(YCacheTL, YEdgeIdx(x, y, zL + 1), 7, 4, V, P);

                        // Z edges
                    case 8:  return MakeAndCache(ZCacheTL, ZEdgeIdx(x, y, zL), 0, 4, V, P);
                    case 9:  return MakeAndCache(ZCacheTL, ZEdgeIdx(x + 1, y, zL), 1, 5, V, P);
                    case 10: return MakeAndCache(ZCacheTL, ZEdgeIdx(x + 1, y + 1, zL), 2, 6, V, P);
                    case 11: return MakeAndCache(ZCacheTL, ZEdgeIdx(x, y + 1, zL), 3, 7, V, P);
                    default: return -1;
                    }
                };

            LM.Triangles.Reserve((Z1 - Z0) * CX * CY * 9);

            for (int32 z = Z0; z < Z1; ++z)
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
                            const int32 eA = (int32)TriTable[CubeIndex][i];
                            const int32 eB = (int32)TriTable[CubeIndex][i + 1];
                            const int32 eC = (int32)TriTable[CubeIndex][i + 2];

                            const int32 iA = GetEdgeVertexIndex(x, y, z, eA, V, P);
                            const int32 iB = GetEdgeVertexIndex(x, y, z, eB, V, P);
                            const int32 iC = GetEdgeVertexIndex(x, y, z, eC, V, P);

                            if (iA < 0 || iB < 0 || iC < 0) continue;
                            if (iA == iB || iB == iC || iC == iA) continue;

                            LM.Triangles.Add(iA);
                            LM.Triangles.Add(iC);
                            LM.Triangles.Add(iB);

                            const FVector A = LM.Vertices[iA];
                            const FVector B = LM.Vertices[iB];
                            const FVector C = LM.Vertices[iC];

                            FVector FaceN = FVector::CrossProduct(C - A, B - A);
                            if (!FaceN.IsNearlyZero())
                            {
                                FaceN.Normalize();
                                LM.Normals[iA] += FaceN;
                                LM.Normals[iB] += FaceN;
                                LM.Normals[iC] += FaceN;
                            }
                        }
                    }
                }
            }

            for (int32 i = 0; i < LM.Normals.Num(); ++i)
            {
                FVector N = LM.Normals[i];
                if (!N.Normalize()) N = FVector::UpVector;
                LM.Normals[i] = N;
            }
        });

    // --------------------------
    // Merge slabs
    // --------------------------
    TArray<int32> SlabBase;
    SlabBase.SetNum(SlabCount + 1);
    SlabBase[0] = 0;

    int32 TotalV = 0, TotalT = 0;
    for (const FLocalMesh& LM : SlabMeshes)
    {
        TotalV += LM.Vertices.Num();
        TotalT += LM.Triangles.Num();
    }

    Out.Vertices.Reserve(TotalV);
    Out.Normals.Reserve(TotalV);
    Out.UV0.Reserve(TotalV);
    Out.Tangents.Reserve(TotalV);
    Out.Triangles.Reserve(TotalT);

    for (int32 s = 0; s < SlabCount; ++s)
    {
        const FLocalMesh& LM = SlabMeshes[s];
        const int32 V0 = Out.Vertices.Num();

        Out.Vertices.Append(LM.Vertices);
        Out.Normals.Append(LM.Normals);
        Out.UV0.Append(LM.UV0);

        const int32 AddCount = LM.Vertices.Num();
        for (int32 i = 0; i < AddCount; ++i)
            Out.Tangents.Add(FProcMeshTangent());

        for (int32 i = 0; i < LM.Triangles.Num(); ++i)
            Out.Triangles.Add(V0 + LM.Triangles[i]);

        SlabBase[s + 1] = Out.Vertices.Num();
    }

    {
        const int32 TotalVerts = Out.Vertices.Num();

        TArray<int32> Parent;
        Parent.SetNumUninitialized(TotalVerts);
        for (int32 i = 0; i < TotalVerts; ++i) Parent[i] = i;

        auto FindRoot = [&](int32 i) -> int32
            {
                int32 r = i;
                while (Parent[r] != r) r = Parent[r];

                while (Parent[i] != i)
                {
                    const int32 p = Parent[i];
                    Parent[i] = r;
                    i = p;
                }
                return r;
            };

        auto UnionTo = [&](int32 Dup, int32 Keep)
            {
                const int32 rDup = FindRoot(Dup);
                const int32 rKeep = FindRoot(Keep);
                if (rDup != rKeep)
                {
                    Parent[rDup] = rKeep;
                }
            };

        for (int32 s = 0; s < SlabCount - 1; ++s)
        {
            const int32 A0 = SlabBase[s];
            const int32 A1 = SlabBase[s + 1];
            const int32 B0 = SlabBase[s + 1];
            const int32 B1 = SlabBase[s + 2];

            TMap<FQuantKey, int32> Map;
            Map.Reserve(A1 - A0);

            for (int32 i = A0; i < A1; ++i)
            {
                Map.Add(QuantizePos01mm(Out.Vertices[i]), i);
            }

            for (int32 i = B0; i < B1; ++i)
            {
                if (int32* Found = Map.Find(QuantizePos01mm(Out.Vertices[i])))
                {
                    UnionTo(i, *Found);
                }
            }
        }

        for (int32 i = 0; i < TotalVerts; ++i)
        {
            const int32 r = FindRoot(i);
            if (r != i)
            {
                Out.Normals[r] += Out.Normals[i];
                Out.Normals[i] = FVector::ZeroVector;
            }
        }

        for (int32& T : Out.Triangles)
        {
            T = FindRoot(T);
        }
    }

    for (int32 i = 0; i < Out.Normals.Num(); ++i)
    {
        FVector N = Out.Normals[i];
        if (!N.Normalize()) N = FVector::UpVector;
        Out.Normals[i] = N;
        Out.Tangents[i] = MakeTangentFromNormal(N);
    }
}