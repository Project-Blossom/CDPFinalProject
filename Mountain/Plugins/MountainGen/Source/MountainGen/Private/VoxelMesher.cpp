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

    static FORCEINLINE void MakeTerrainUVAndTangent(
        const FVector& PLocalCm,
        const FVector& N,
        FVector2D& OutUV,
        FProcMeshTangent& OutTangent)
    {
        constexpr float UVScale = 0.001f;

        const FVector AbsN(FMath::Abs(N.X), FMath::Abs(N.Y), FMath::Abs(N.Z));

        FVector TangentX = FVector::RightVector;
        FVector DesiredTangentY = FVector::ForwardVector;

        if (AbsN.Z >= AbsN.X && AbsN.Z >= AbsN.Y)
        {
            // 상단/완만한 면: 월드 XY 투영
            // U = +X, V = +Y
            OutUV = FVector2D(PLocalCm.X * UVScale, PLocalCm.Y * UVScale);
            TangentX = FVector::RightVector;
            DesiredTangentY = FVector::ForwardVector;
        }
        else if (AbsN.X >= AbsN.Y)
        {
            // X 방향을 보는 절벽: YZ 투영
            // U = +Y, V = +Z
            OutUV = FVector2D(PLocalCm.Y * UVScale, PLocalCm.Z * UVScale);
            TangentX = FVector::ForwardVector;
            DesiredTangentY = FVector::UpVector;
        }
        else
        {
            // Y 방향을 보는 절벽: XZ 투영
            // U = +X, V = +Z
            OutUV = FVector2D(PLocalCm.X * UVScale, PLocalCm.Z * UVScale);
            TangentX = FVector::RightVector;
            DesiredTangentY = FVector::UpVector;
        }

        TangentX = TangentX - FVector::DotProduct(TangentX, N) * N;
        if (!TangentX.Normalize())
        {
            TangentX = FVector::CrossProduct(FVector::UpVector, N);
            if (!TangentX.Normalize())
            {
                TangentX = FVector::CrossProduct(FVector::RightVector, N);
                if (!TangentX.Normalize())
                {
                    TangentX = FVector::ForwardVector;
                }
            }
        }

        DesiredTangentY = DesiredTangentY - FVector::DotProduct(DesiredTangentY, N) * N;
        DesiredTangentY.Normalize();

        FVector BuiltTangentY = FVector::CrossProduct(N, TangentX);
        bool bFlipTangentY = false;
        if (BuiltTangentY.Normalize() && DesiredTangentY.IsNormalized())
        {
            bFlipTangentY = FVector::DotProduct(BuiltTangentY, DesiredTangentY) < 0.f;
        }

        OutTangent = FProcMeshTangent(TangentX, bFlipTangentY);
    }

    static FORCEINLINE float SampleChunkDensityNearestSafe(
        const FVoxelChunk& Chunk,
        int32 X,
        int32 Y,
        int32 Z)
    {
        X = FMath::Clamp(X, 0, Chunk.SizeX - 1);
        Y = FMath::Clamp(Y, 0, Chunk.SizeY - 1);
        Z = FMath::Clamp(Z, 0, Chunk.SizeZ - 1);
        return Chunk.Density[Idx3(X, Y, Z, Chunk.SizeX, Chunk.SizeY)];
    }

    static FORCEINLINE float SampleChunkDensityTrilinearSafe(
        const FVoxelChunk& Chunk,
        const FVector& GridPos)
    {
        if (Chunk.SizeX <= 0 || Chunk.SizeY <= 0 || Chunk.SizeZ <= 0 || Chunk.Density.Num() <= 0)
        {
            return 0.0f;
        }

        const float FX = FMath::Clamp(GridPos.X, 0.0f, static_cast<float>(Chunk.SizeX - 1));
        const float FY = FMath::Clamp(GridPos.Y, 0.0f, static_cast<float>(Chunk.SizeY - 1));
        const float FZ = FMath::Clamp(GridPos.Z, 0.0f, static_cast<float>(Chunk.SizeZ - 1));

        const int32 X0 = FMath::FloorToInt(FX);
        const int32 Y0 = FMath::FloorToInt(FY);
        const int32 Z0 = FMath::FloorToInt(FZ);

        const int32 X1 = FMath::Min(X0 + 1, Chunk.SizeX - 1);
        const int32 Y1 = FMath::Min(Y0 + 1, Chunk.SizeY - 1);
        const int32 Z1 = FMath::Min(Z0 + 1, Chunk.SizeZ - 1);

        const float TX = FX - static_cast<float>(X0);
        const float TY = FY - static_cast<float>(Y0);
        const float TZ = FZ - static_cast<float>(Z0);

        const float C000 = SampleChunkDensityNearestSafe(Chunk, X0, Y0, Z0);
        const float C100 = SampleChunkDensityNearestSafe(Chunk, X1, Y0, Z0);
        const float C010 = SampleChunkDensityNearestSafe(Chunk, X0, Y1, Z0);
        const float C110 = SampleChunkDensityNearestSafe(Chunk, X1, Y1, Z0);
        const float C001 = SampleChunkDensityNearestSafe(Chunk, X0, Y0, Z1);
        const float C101 = SampleChunkDensityNearestSafe(Chunk, X1, Y0, Z1);
        const float C011 = SampleChunkDensityNearestSafe(Chunk, X0, Y1, Z1);
        const float C111 = SampleChunkDensityNearestSafe(Chunk, X1, Y1, Z1);

        const float C00 = FMath::Lerp(C000, C100, TX);
        const float C10 = FMath::Lerp(C010, C110, TX);
        const float C01 = FMath::Lerp(C001, C101, TX);
        const float C11 = FMath::Lerp(C011, C111, TX);

        const float C0 = FMath::Lerp(C00, C10, TY);
        const float C1 = FMath::Lerp(C01, C11, TY);

        return FMath::Lerp(C0, C1, TZ);
    }

    static FORCEINLINE FVector EstimateOutwardNormalFromChunkDensity(
        const FVoxelChunk& Chunk,
        float VoxelSizeCm,
        const FVector& ChunkOriginWorld,
        const FVector& WorldPosCm)
    {
        if (VoxelSizeCm <= KINDA_SMALL_NUMBER)
        {
            return FVector::ZeroVector;
        }

        const FVector GridPos = (WorldPosCm - ChunkOriginWorld) / VoxelSizeCm;

        const float E = 0.5f;
        const float Dx = SampleChunkDensityTrilinearSafe(Chunk, GridPos + FVector(E, 0.f, 0.f))
            - SampleChunkDensityTrilinearSafe(Chunk, GridPos - FVector(E, 0.f, 0.f));
        const float Dy = SampleChunkDensityTrilinearSafe(Chunk, GridPos + FVector(0.f, E, 0.f))
            - SampleChunkDensityTrilinearSafe(Chunk, GridPos - FVector(0.f, E, 0.f));
        const float Dz = SampleChunkDensityTrilinearSafe(Chunk, GridPos + FVector(0.f, 0.f, E))
            - SampleChunkDensityTrilinearSafe(Chunk, GridPos - FVector(0.f, 0.f, E));

        FVector Gradient(Dx, Dy, Dz);
        if (!Gradient.Normalize())
        {
            return FVector::ZeroVector;
        }

        return Gradient;
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
        TArray<FLinearColor>      Colors;
        TArray<FProcMeshTangent>  Tangents;

        struct FBoundaryV
        {
            FQuantKey Key;
            int32 LocalIndex = -1;
        };
        TArray<FBoundaryV> LowerBoundary;
        TArray<FBoundaryV> UpperBoundary;
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
    const FVoxelDensityGenerator& Gen,
    FChunkMeshData& Out)
{
    Out.Vertices.Reset();
    Out.Triangles.Reset();
    Out.Normals.Reset();
    Out.UV0.Reset();
    Out.Colors.Reset();
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
            LM.Colors.Reset();
            LM.Tangents.Reset();
            LM.LowerBoundary.Reset();
            LM.UpperBoundary.Reset();

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

            auto AddSharedVertex = [&](const FVector& PWorld, bool bLower, bool bUpper) -> int32
                {
                    const int32 Idx = LM.Vertices.Num();
                    const FVector PLocal = (PWorld - ActorWorld);
                    LM.Vertices.Add(PLocal);

                    LM.Normals.Add(FVector::ZeroVector);
                    LM.UV0.Add(FVector2D::ZeroVector);
                    LM.Colors.Add(FLinearColor::White);
                    LM.Tangents.Add(FProcMeshTangent());


                    if (bLower || bUpper)
                    {
                        FLocalMesh::FBoundaryV BV;
                        BV.Key = QuantizePos01mm(PLocal);
                        BV.LocalIndex = Idx;
                        if (bLower) LM.LowerBoundary.Add(BV);
                        if (bUpper) LM.UpperBoundary.Add(BV);
                    }
                    return Idx;
                };

            auto MakeAndCache = [&](FStampedEdgeCache& Cache, int32 Slot, int32 cA, int32 cB, const float V[8], const FVector P[8], bool bLower, bool bUpper) -> int32
                {
                    Cache.TouchSlot(Slot, BuildStamp);
                    int32& CachedIdx = Cache.SlotIndex(Slot);
                    if (CachedIdx != -1) return CachedIdx;

                    const FVector PV = LerpVertex(P[cA], P[cB], V[cA], V[cB], IsoLevel);
                    CachedIdx = AddSharedVertex(PV, bLower, bUpper);
                    return CachedIdx;
                };

            auto GetEdgeVertexIndex = [&](int32 x, int32 y, int32 zCell, int32 e, const float V[8], const FVector P[8]) -> int32
                {
                    const int32 zL = zCell - Z0;
                    const int32 zPlaneLower = 0;
                    const int32 zPlaneUpper = ZPlaneCount - 1;

                    switch (e)
                    {
                        // X edges
                    case 0:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y, zL), 0, 1, V, P, zL == zPlaneLower, zL == zPlaneUpper);
                    case 2:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y + 1, zL), 2, 3, V, P, zL == zPlaneLower, zL == zPlaneUpper);
                    case 4:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y, zL + 1), 4, 5, V, P, (zL + 1) == zPlaneLower, (zL + 1) == zPlaneUpper);
                    case 6:  return MakeAndCache(XCacheTL, XEdgeIdx(x, y + 1, zL + 1), 6, 7, V, P, (zL + 1) == zPlaneLower, (zL + 1) == zPlaneUpper);

                        // Y edges
                    case 1:  return MakeAndCache(YCacheTL, YEdgeIdx(x + 1, y, zL), 1, 2, V, P, zL == zPlaneLower, zL == zPlaneUpper);
                    case 3:  return MakeAndCache(YCacheTL, YEdgeIdx(x, y, zL), 3, 0, V, P, zL == zPlaneLower, zL == zPlaneUpper);
                    case 5:  return MakeAndCache(YCacheTL, YEdgeIdx(x + 1, y, zL + 1), 5, 6, V, P, (zL + 1) == zPlaneLower, (zL + 1) == zPlaneUpper);
                    case 7:  return MakeAndCache(YCacheTL, YEdgeIdx(x, y, zL + 1), 7, 4, V, P, (zL + 1) == zPlaneLower, (zL + 1) == zPlaneUpper);

                        // Z edges
                    case 8:  return MakeAndCache(ZCacheTL, ZEdgeIdx(x, y, zL), 0, 4, V, P, false, false);
                    case 9:  return MakeAndCache(ZCacheTL, ZEdgeIdx(x + 1, y, zL), 1, 5, V, P, false, false);
                    case 10: return MakeAndCache(ZCacheTL, ZEdgeIdx(x + 1, y + 1, zL), 2, 6, V, P, false, false);
                    case 11: return MakeAndCache(ZCacheTL, ZEdgeIdx(x, y + 1, zL), 3, 7, V, P, false, false);
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

                        if (V[0] > IsoLevel) CubeIndex |= 1;
                        if (V[1] > IsoLevel) CubeIndex |= 2;
                        if (V[2] > IsoLevel) CubeIndex |= 4;
                        if (V[3] > IsoLevel) CubeIndex |= 8;
                        if (V[4] > IsoLevel) CubeIndex |= 16;
                        if (V[5] > IsoLevel) CubeIndex |= 32;
                        if (V[6] > IsoLevel) CubeIndex |= 64;
                        if (V[7] > IsoLevel) CubeIndex |= 128;

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
                            LM.Triangles.Add(iB);
                            LM.Triangles.Add(iC);

                            const FVector A = LM.Vertices[iA];
                            const FVector B = LM.Vertices[iB];
                            const FVector C = LM.Vertices[iC];

                            FVector FaceN = FVector::CrossProduct(B - A, C - A);
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

    int32 TotalV = 0, TotalT = 0;
    for (const FLocalMesh& LM : SlabMeshes)
    {
        TotalV += LM.Vertices.Num();
        TotalT += LM.Triangles.Num();
    }

    Out.Vertices.Reserve(TotalV);
    Out.Normals.Reserve(TotalV);
    Out.UV0.Reserve(TotalV);
    Out.Colors.Reserve(TotalV);
    Out.Tangents.Reserve(TotalV);
    Out.Triangles.Reserve(TotalT);

    TMap<FQuantKey, int32> PrevUpperMap;

    for (int32 s = 0; s < SlabCount; ++s)
    {
        const FLocalMesh& LM = SlabMeshes[s];
        const int32 VB = LM.Vertices.Num();
        if (VB == 0 || LM.Triangles.Num() == 0)
        {
            PrevUpperMap.Reset();
            continue;
        }

        TArray<int32> LocalToGlobal;
        LocalToGlobal.Init(-1, VB);

        if (s > 0 && PrevUpperMap.Num() > 0 && LM.LowerBoundary.Num() > 0)
        {
            for (const FLocalMesh::FBoundaryV& BV : LM.LowerBoundary)
            {
                if ((uint32)BV.LocalIndex >= (uint32)VB) continue;
                if (int32* Found = PrevUpperMap.Find(BV.Key))
                {
                    LocalToGlobal[BV.LocalIndex] = *Found;
                }
            }
        }

        for (int32 i = 0; i < VB; ++i)
        {
            const int32 Existing = LocalToGlobal[i];
            if (Existing >= 0)
            {
                if (LM.Normals.IsValidIndex(i))
                    Out.Normals[Existing] += LM.Normals[i];
                continue;
            }

            const int32 NewIdx = Out.Vertices.Num();
            LocalToGlobal[i] = NewIdx;
            Out.Vertices.Add(LM.Vertices[i]);
            Out.Normals.Add(LM.Normals.IsValidIndex(i) ? LM.Normals[i] : FVector::ZeroVector);
            Out.UV0.Add(LM.UV0.IsValidIndex(i) ? LM.UV0[i] : FVector2D::ZeroVector);
            Out.Colors.Add(LM.Colors.IsValidIndex(i) ? LM.Colors[i] : FLinearColor::White);
            Out.Tangents.Add(FProcMeshTangent());
        }

        for (int32 ti = 0; ti < LM.Triangles.Num(); ++ti)
        {
            const int32 li = LM.Triangles[ti];
            if ((uint32)li >= (uint32)VB) continue;
            const int32 gi = LocalToGlobal[li];
            if (gi >= 0) Out.Triangles.Add(gi);
        }

        PrevUpperMap.Reset();
        PrevUpperMap.Reserve(LM.UpperBoundary.Num());
        for (const FLocalMesh::FBoundaryV& BV : LM.UpperBoundary)
        {
            if ((uint32)BV.LocalIndex >= (uint32)VB) continue;
            const int32 GI = LocalToGlobal[BV.LocalIndex];
            if (GI >= 0) PrevUpperMap.Add(BV.Key, GI);
        }
    }

    Out.Colors.SetNum(Out.Vertices.Num());

    for (int32 i = 0; i < Out.Vertices.Num(); ++i)
    {
        const FVector WorldPos = ActorWorld + Out.Vertices[i];

        FVector LightingN = Out.Normals.IsValidIndex(i) ? Out.Normals[i] : FVector::ZeroVector;
        if (!LightingN.Normalize())
        {
            LightingN = FVector::UpVector;
        }

        FVector SlopeN = EstimateOutwardNormalFromChunkDensity(Chunk, VoxelSizeCm, ChunkOriginWorld, WorldPos);
        if (!SlopeN.Normalize())
        {
            SlopeN = LightingN;
        }

        FVector PlateauN = FVector::ZeroVector;
        float PlateauWeight = 0.f;
        if (Gen.TryComputeTopPlateauSurfaceNormal(WorldPos, VoxelSizeCm * 2.25f, PlateauN, PlateauWeight))
        {
            PlateauN.Normalize();

            FVector BlendedN = FMath::Lerp(LightingN, PlateauN, FMath::Clamp(PlateauWeight, 0.f, 1.f));
            if (BlendedN.Normalize())
            {
                LightingN = BlendedN;
            }
        }
        else if (FVector::DotProduct(LightingN, SlopeN) < -0.15f)
        {
            LightingN *= -1.f;
        }

        const float EncodedSlopeZ = FMath::Clamp(SlopeN.Z * 0.5f + 0.5f, 0.f, 1.f);
        Out.Colors[i] = FLinearColor(EncodedSlopeZ, 1.f, 1.f, 1.f);

        Out.Normals[i] = LightingN;

        FVector2D UV;
        FProcMeshTangent Tangent;
        MakeTerrainUVAndTangent(Out.Vertices[i], LightingN, UV, Tangent);

        if (Out.UV0.IsValidIndex(i))
        {
            Out.UV0[i] = UV;
        }
        if (Out.Tangents.IsValidIndex(i))
        {
            Out.Tangents[i] = Tangent;
        }
    }
}