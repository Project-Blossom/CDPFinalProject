#include "MountainGenWorldActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "Misc/Crc.h"

#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "Math/RandomStream.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"
#include "MountainGenAutoTune.h"

// ============================================================
// Weld
// ============================================================

struct FMGWeldKey
{
    int64 X, Y, Z;

    bool operator==(const FMGWeldKey& O) const
    {
        return X == O.X && Y == O.Y && Z == O.Z;
    }
};

FORCEINLINE uint32 GetTypeHash(const FMGWeldKey& K)
{
    uint32 H = ::GetTypeHash(K.X);
    H = HashCombine(H, ::GetTypeHash(K.Y));
    H = HashCombine(H, ::GetTypeHash(K.Z));
    return H;
}

static void MG_WeldVertices_Quantized(FChunkMeshData& M, float EpsilonCm)
{
    const int32 V = M.Vertices.Num();
    const int32 I = M.Triangles.Num();
    if (V <= 0 || I <= 0) return;

    EpsilonCm = FMath::Max(0.01f, EpsilonCm);

    TArray<int32> Rep;
    Rep.Init(-1, V);

    TArray<FVector> NewVerts;
    TArray<FVector> NewNormals;
    TArray<FVector2D> NewUV0;
    TArray<FProcMeshTangent> NewTangents;

    NewVerts.Reserve(V);
    NewNormals.Reserve(V);
    NewUV0.Reserve(V);
    NewTangents.Reserve(V);

    TMap<FMGWeldKey, int32> CellToRep;
    CellToRep.Reserve(V);

    auto QuantKey = [&](const FVector& P) -> FMGWeldKey
        {
            const double Inv = 1.0 / (double)EpsilonCm;
            return FMGWeldKey{
                FMath::RoundToInt64((double)P.X * Inv),
                FMath::RoundToInt64((double)P.Y * Inv),
                FMath::RoundToInt64((double)P.Z * Inv)
            };
        };

    for (int32 i = 0; i < V; ++i)
    {
        const FVector P = M.Vertices[i];
        const FMGWeldKey K = QuantKey(P);

        int32* Found = CellToRep.Find(K);
        if (!Found)
        {
            const int32 NewIdx = NewVerts.Num();
            CellToRep.Add(K, NewIdx);

            Rep[i] = NewIdx;
            NewVerts.Add(P);

            const FVector N = M.Normals.IsValidIndex(i) ? M.Normals[i] : FVector::UpVector;
            NewNormals.Add(N);

            const FVector2D UV = M.UV0.IsValidIndex(i) ? M.UV0[i] : FVector2D::ZeroVector;
            NewUV0.Add(UV);

            const FProcMeshTangent Tng = M.Tangents.IsValidIndex(i) ? M.Tangents[i] : FProcMeshTangent();
            NewTangents.Add(Tng);
        }
        else
        {
            const int32 RepIdx = *Found;
            Rep[i] = RepIdx;

            const FVector N = M.Normals.IsValidIndex(i) ? M.Normals[i] : FVector::UpVector;
            NewNormals[RepIdx] += N;
        }
    }

    for (int32 r = 0; r < NewNormals.Num(); ++r)
    {
        if (!NewNormals[r].Normalize())
            NewNormals[r] = FVector::UpVector;
    }

    TArray<int32> NewTris;
    NewTris.Reserve(I);

    const int32 NumTris = I / 3;
    for (int32 t = 0; t < NumTris; ++t)
    {
        const int32 a0 = M.Triangles[t * 3 + 0];
        const int32 b0 = M.Triangles[t * 3 + 1];
        const int32 c0 = M.Triangles[t * 3 + 2];

        if ((uint32)a0 >= (uint32)V || (uint32)b0 >= (uint32)V || (uint32)c0 >= (uint32)V)
            continue;

        const int32 a = Rep[a0];
        const int32 b = Rep[b0];
        const int32 c = Rep[c0];

        if (a == b || b == c || c == a)
            continue;

        NewTris.Add(a);
        NewTris.Add(b);
        NewTris.Add(c);
    }

    const int32 NewV = NewVerts.Num();

    TArray<int32> Used;
    Used.Init(0, NewV);
    for (int32 idx : NewTris)
        if ((uint32)idx < (uint32)NewV) Used[idx] = 1;

    TArray<int32> FinalRemap;
    FinalRemap.Init(-1, NewV);

    FChunkMeshData Out;
    Out.Vertices.Reserve(NewV);
    Out.Normals.Reserve(NewV);
    Out.UV0.Reserve(NewV);
    Out.Tangents.Reserve(NewV);

    for (int32 r = 0; r < NewV; ++r)
    {
        if (!Used[r]) continue;
        FinalRemap[r] = Out.Vertices.Num();
        Out.Vertices.Add(NewVerts[r]);
        Out.Normals.Add(NewNormals[r]);
        Out.UV0.Add(NewUV0[r]);
        Out.Tangents.Add(NewTangents[r]);
    }

    Out.Triangles.Reserve(NewTris.Num());
    for (int32 idx : NewTris)
    {
        const int32 ni = FinalRemap[idx];
        if (ni >= 0) Out.Triangles.Add(ni);
    }

    M = MoveTemp(Out);
}

// ============================================================
// Cull islands
// ============================================================

struct FDSU
{
    TArray<int32> Parent;
    TArray<uint8> Rank;

    explicit FDSU(int32 N)
    {
        Parent.SetNumUninitialized(N);
        Rank.Init(0, N);
        for (int32 i = 0; i < N; ++i) Parent[i] = i;
    }

    int32 Find(int32 x)
    {
        while (Parent[x] != x)
        {
            Parent[x] = Parent[Parent[x]];
            x = Parent[x];
        }
        return x;
    }

    void Union(int32 a, int32 b)
    {
        a = Find(a);
        b = Find(b);
        if (a == b) return;
        if (Rank[a] < Rank[b]) Swap(a, b);
        Parent[b] = a;
        if (Rank[a] == Rank[b]) Rank[a]++;
    }
};

static void MG_CullMeshIslands(FChunkMeshData& Out, int32 MinTrisToKeep, bool bKeepLargestOnly)
{
    const int32 V = Out.Vertices.Num();
    const int32 T = Out.Triangles.Num() / 3;
    if (V == 0 || T == 0) return;

    FDSU Dsu(V);
    auto Valid = [&](int32 i) { return (uint32)i < (uint32)V; };

    for (int32 t = 0; t < T; ++t)
    {
        int32 a = Out.Triangles[t * 3 + 0];
        int32 b = Out.Triangles[t * 3 + 1];
        int32 c = Out.Triangles[t * 3 + 2];
        if (!Valid(a) || !Valid(b) || !Valid(c)) continue;
        Dsu.Union(a, b);
        Dsu.Union(b, c);
        Dsu.Union(c, a);
    }

    TMap<int32, int32> RootCount;
    RootCount.Reserve(T);

    for (int32 t = 0; t < T; ++t)
    {
        int32 a = Out.Triangles[t * 3];
        if (!Valid(a)) continue;
        RootCount.FindOrAdd(Dsu.Find(a))++;
    }

    int32 KeepRoot = -1;
    if (bKeepLargestOnly)
    {
        int32 Best = 0;
        for (auto& K : RootCount)
            if (K.Value > Best) { Best = K.Value; KeepRoot = K.Key; }
    }

    auto Keep = [&](int32 r)
        {
            if (bKeepLargestOnly) return r == KeepRoot;
            const int32* C = RootCount.Find(r);
            return C && *C >= MinTrisToKeep;
        };

    TArray<int32> Remap; Remap.Init(-1, V);
    FChunkMeshData New;

    for (int32 t = 0; t < T; ++t)
    {
        int32 o[3] = {
            Out.Triangles[t * 3 + 0],
            Out.Triangles[t * 3 + 1],
            Out.Triangles[t * 3 + 2]
        };
        if (!Valid(o[0]) || !Valid(o[1]) || !Valid(o[2])) continue;
        if (!Keep(Dsu.Find(o[0]))) continue;

        int32 r[3];
        for (int i = 0; i < 3; i++)
        {
            int32& idx = Remap[o[i]];
            if (idx == -1)
            {
                idx = New.Vertices.Num();
                New.Vertices.Add(Out.Vertices[o[i]]);
                New.Normals.Add(Out.Normals.IsValidIndex(o[i]) ? Out.Normals[o[i]] : FVector::UpVector);
                New.UV0.Add(Out.UV0.IsValidIndex(o[i]) ? Out.UV0[o[i]] : FVector2D::ZeroVector);
                New.Tangents.Add(Out.Tangents.IsValidIndex(o[i]) ? Out.Tangents[o[i]] : FProcMeshTangent());
            }
            r[i] = idx;
        }
        New.Triangles.Append({ r[0], r[1], r[2] });
    }

    Out = MoveTemp(New);
}


// ============================================================
// Top flat plateau mesh builder
// ============================================================

struct FMGCliffRimSample
{
    float Y = 0.f;
    float X = 0.f;
    float Z = 0.f;
    FVector Normal = FVector::UpVector;
    bool bValid = false;
};

static bool MG_IsPlateauRimCandidateTriangle(
    const FChunkMeshData& CliffMesh,
    int32 TriIndex,
    float ZTop,
    float MinZ,
    float MaxZ,
    FVector& OutCenter,
    FVector& OutNormal)
{
    const int32 IA = CliffMesh.Triangles[TriIndex * 3 + 0];
    const int32 IB = CliffMesh.Triangles[TriIndex * 3 + 1];
    const int32 IC = CliffMesh.Triangles[TriIndex * 3 + 2];

    if (!CliffMesh.Vertices.IsValidIndex(IA) ||
        !CliffMesh.Vertices.IsValidIndex(IB) ||
        !CliffMesh.Vertices.IsValidIndex(IC))
    {
        return false;
    }

    const FVector A = CliffMesh.Vertices[IA];
    const FVector B = CliffMesh.Vertices[IB];
    const FVector C = CliffMesh.Vertices[IC];

    const float TriMaxZ = FMath::Max(A.Z, FMath::Max(B.Z, C.Z));
    const float TriMinZ = FMath::Min(A.Z, FMath::Min(B.Z, C.Z));

    if (TriMaxZ < MinZ || TriMinZ > MaxZ)
    {
        return false;
    }

    FVector N = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
    if (N.IsNearlyZero())
    {
        return false;
    }

    if (CliffMesh.Normals.IsValidIndex(IA) && CliffMesh.Normals.IsValidIndex(IB) && CliffMesh.Normals.IsValidIndex(IC))
    {
        const FVector AvgN = (CliffMesh.Normals[IA] + CliffMesh.Normals[IB] + CliffMesh.Normals[IC]).GetSafeNormal();
        if (!AvgN.IsNearlyZero())
        {
            N = AvgN;
        }
    }

    // Plateau 접합선은 "상단 림"이어야 한다.
    // 아래쪽 오버행, 동굴 안쪽, 수직 절벽 중간면까지 접합 후보로 쓰면
    // Plateau가 등반면을 덮어버린다.
    // 그래서 상단 근처이면서 아래를 향한 면은 제외한다.
    if (N.Z < -0.15f)
    {
        return false;
    }

    OutCenter = (A + B + C) / 3.f;
    OutNormal = N;
    return true;
}

static void MG_BuildCliffRimSamples(
    const FChunkMeshData& CliffMesh,
    const FMountainGenSettings& S,
    float ZTop,
    float YLeft,
    float YRight,
    int32 SegY,
    TArray<FMGCliffRimSample>& OutRim)
{
    OutRim.Reset();
    OutRim.SetNum(SegY + 1);

    const float Band = FMath::Max(10.f, S.TopPlateauConformSampleBandCm);
    const float SearchDepth = FMath::Max(100.f, S.TopPlateauConformSearchDepthCm);
    const float MinZ = ZTop - SearchDepth;
    const float MaxZ = ZTop + FMath::Max(100.f, S.VoxelSizeCm * 2.f);
    const float MaxDrop = FMath::Max(100.f, S.TopPlateauMaxConformDropCm);
    const float ContactOverlapDown = FMath::Max(0.f, S.TopPlateauContactOverlapDownCm);
    const float RimZBand = FMath::Max(S.VoxelSizeCm * 1.5f, SearchDepth * 0.10f);

    const int32 TriCount = CliffMesh.Triangles.Num() / 3;

    for (int32 iy = 0; iy <= SegY; ++iy)
    {
        const float AY = (float)iy / (float)SegY;
        const float QueryY = FMath::Lerp(YLeft, YRight, AY);

        FMGCliffRimSample Sample;
        Sample.Y = QueryY;

        // 1단계: 해당 Y 단면에서 가장 높은 상단 영역을 먼저 찾는다.
        // 이 값이 있어야 아래쪽 노이즈/오버행을 접합 기준으로 잘못 잡지 않는다.
        bool bHasTop = false;
        float HighestZ = -FLT_MAX;

        for (int32 t = 0; t < TriCount; ++t)
        {
            FVector Center;
            FVector Normal;
            if (!MG_IsPlateauRimCandidateTriangle(CliffMesh, t, ZTop, MinZ, MaxZ, Center, Normal))
            {
                continue;
            }

            const float YDist = FMath::Abs(Center.Y - QueryY);
            if (YDist > Band)
            {
                continue;
            }

            if (!bHasTop || Center.Z > HighestZ)
            {
                bHasTop = true;
                HighestZ = Center.Z;
            }
        }

        if (!bHasTop)
        {
            OutRim[iy] = Sample;
            continue;
        }

        // 2단계: 가장 높은 상단 림 주변에서만, 같은 Y 단면의 가로 끝 X를 고른다.
        // 전체 폭의 가장 먼 X를 공유하지 않고, 각 세로 단면의 림 끝점만 쓴다.
        bool bFound = false;
        float BestX = -FLT_MAX;
        float BestZ = HighestZ;
        float BestYDist = FLT_MAX;
        FVector BestNormal = FVector::UpVector;

        const float RimMinZ = FMath::Max(ZTop - MaxDrop, HighestZ - RimZBand);

        for (int32 t = 0; t < TriCount; ++t)
        {
            FVector Center;
            FVector Normal;
            if (!MG_IsPlateauRimCandidateTriangle(CliffMesh, t, ZTop, MinZ, MaxZ, Center, Normal))
            {
                continue;
            }

            const float YDist = FMath::Abs(Center.Y - QueryY);
            if (YDist > Band)
            {
                continue;
            }

            if (Center.Z < RimMinZ)
            {
                continue;
            }

            // 같은 Y 세로 단면에서 Plateau가 시작될 가로 끝을 고른다.
            // X를 우선하고, 거의 같은 X라면 Y가 더 가까운 것과 더 높은 림을 선호한다.
            const bool bBetterX = Center.X > BestX + 1.f;
            const bool bSimilarX = FMath::Abs(Center.X - BestX) <= 1.f;
            const bool bBetterY = YDist < BestYDist - 1.f;
            const bool bBetterZ = Center.Z > BestZ + 1.f;

            if (!bFound || bBetterX || (bSimilarX && (bBetterY || (FMath::Abs(YDist - BestYDist) <= 1.f && bBetterZ))))
            {
                bFound = true;
                BestX = Center.X;
                BestZ = Center.Z;
                BestYDist = YDist;
                BestNormal = Normal;
            }
        }

        if (bFound)
        {
            Sample.X = BestX + FMath::Max(0.f, S.TopPlateauFrontOverlapCm);
            Sample.Z = FMath::Clamp(BestZ - ContactOverlapDown, ZTop - MaxDrop, ZTop + FMath::Max(100.f, S.VoxelSizeCm * 2.f));
            Sample.Normal = BestNormal.GetSafeNormal();
            Sample.bValid = true;
        }

        OutRim[iy] = Sample;
    }

    // 3단계: 단면별 림이 너무 튀면 Plateau 앞 경계가 찢어져 보인다.
    // 단, 전체 X로 통일하지 않는다. 국소적인 튐만 3점 median으로 완화한다.
    for (int32 Pass = 0; Pass < 2; ++Pass)
    {
        TArray<FMGCliffRimSample> Prev = OutRim;

        for (int32 iy = 1; iy < SegY; ++iy)
        {
            if (!Prev[iy - 1].bValid || !Prev[iy].bValid || !Prev[iy + 1].bValid)
            {
                continue;
            }

            const float X0 = Prev[iy - 1].X;
            const float X1 = Prev[iy].X;
            const float X2 = Prev[iy + 1].X;
            const float Z0 = Prev[iy - 1].Z;
            const float Z1 = Prev[iy].Z;
            const float Z2 = Prev[iy + 1].Z;

            const float LocalX = (X0 + X1 + X2) / 3.f;
            const float LocalZ = (Z0 + Z1 + Z2) / 3.f;

            const float SpikeLimitX = FMath::Max(S.VoxelSizeCm * 3.f, Band * 1.5f);
            const float SpikeLimitZ = FMath::Max(S.VoxelSizeCm * 2.f, S.TopPlateauMaxConformDropCm * 0.25f);

            if (FMath::Abs(X1 - LocalX) > SpikeLimitX)
            {
                OutRim[iy].X = LocalX;
            }

            if (FMath::Abs(Z1 - LocalZ) > SpikeLimitZ)
            {
                OutRim[iy].Z = LocalZ;
            }
        }
    }
}


static float MG_PlateauSmooth01(float T)
{
    T = FMath::Clamp(T, 0.f, 1.f);
    return T * T * (3.f - 2.f * T);
}

static float MG_PlateauFBM2D(const FVector2D& P, int32 Octaves, float Lacunarity = 2.02f, float Gain = 0.5f)
{
    float Sum = 0.f;
    float Amp = 0.5f;
    float Freq = 1.f;
    float Norm = 0.f;

    Octaves = FMath::Clamp(Octaves, 1, 8);
    for (int32 i = 0; i < Octaves; ++i)
    {
        Sum += FMath::PerlinNoise2D(P * Freq) * Amp;
        Norm += Amp;
        Amp *= Gain;
        Freq *= Lacunarity;
    }

    return (Norm > KINDA_SMALL_NUMBER) ? (Sum / Norm) : 0.f;
}

static float MG_PlateauEdgeFade(float X, float Y, float XBack, float XFront, float YLeft, float YRight, const FMountainGenSettings& S)
{
    const float FadeCm = FMath::Max(0.f, S.PlateauSurfaceEdgeFadeCm);
    if (FadeCm <= KINDA_SMALL_NUMBER)
    {
        return 1.f;
    }

    const float DistBack = FMath::Max(0.f, X - XBack);
    const float DistFront = FMath::Max(0.f, XFront - X);
    const float DistLeft = FMath::Max(0.f, Y - YLeft);
    const float DistRight = FMath::Max(0.f, YRight - Y);

    const float EdgeDist = FMath::Min(FMath::Min(DistBack, DistFront), FMath::Min(DistLeft, DistRight));
    return MG_PlateauSmooth01(EdgeDist / FadeCm);
}

static float MG_PlateauUpliftNoiseCm(const FMountainGenSettings& S, int32 PlateauSeed, float X, float Y)
{
    const float Strength = FMath::Max(0.f, S.PlateauSurfaceNoiseStrengthCm);
    if (Strength <= KINDA_SMALL_NUMBER)
    {
        return 0.f;
    }

    const float Scale = FMath::Max(1000.f, S.PlateauSurfaceNoiseScaleCm);
    const int32 Octaves = FMath::Clamp(S.PlateauSurfaceNoiseOctaves, 1, 8);
    const FVector2D SeedShift((float)(PlateauSeed % 7919) * 0.017f, (float)(PlateauSeed % 3571) * 0.023f);

    FVector2D P(X / Scale, Y / Scale);

    // 일반 Landscape/Heightfield처럼 좌표를 약간 휘어서 반복적인 능선/격자감을 줄인다.
    const float WarpScale = Scale * 1.85f;
    const FVector2D WP(X / WarpScale, Y / WarpScale);
    const float WarpX = MG_PlateauFBM2D(WP + SeedShift + FVector2D(17.2f, 3.1f), 3);
    const float WarpY = MG_PlateauFBM2D(WP + SeedShift + FVector2D(-5.4f, 11.7f), 3);
    P += FVector2D(WarpX, WarpY) * 0.42f;

    // 큰 낮은 언덕 + 중간 굴곡 + 작은 표면 변화.
    const float Macro = MG_PlateauFBM2D(P * 0.72f + SeedShift, FMath::Max(1, Octaves - 2));
    const float Mid = MG_PlateauFBM2D(P * 1.55f + SeedShift + FVector2D(31.4f, -9.8f), FMath::Max(1, Octaves - 1));
    const float Micro = MG_PlateauFBM2D(P * 4.25f + SeedShift + FVector2D(-12.6f, 44.3f), FMath::Max(1, Octaves - 2));

    // 아래로 파이지 않게 한다. 단순 max(noise,0)가 아니라 SmoothStep/Pow로 낮은 언덕만 부드럽게 남긴다.
    float Hill = Macro * 0.72f + Mid * 0.23f + Micro * 0.05f;
    Hill = (Hill + 1.f) * 0.5f;       // 0..1
    Hill = MG_PlateauSmooth01(Hill);  // 부드러운 언덕화
    Hill = FMath::Pow(Hill, 1.35f);   // 평면 위로 완만하게 솟는 느낌

    return Hill * Strength;
}

static FVector2D MG_MakePlateauMaterialUV(const FMountainGenSettings& S, float X, float Y)
{
    const float ScaleCm = FMath::Max(100.f, S.PlateauMaterialUVScaleCm);
    const float Rad = FMath::DegreesToRadians(S.PlateauMaterialUVRotationDeg);
    const float C = FMath::Cos(Rad);
    const float SN = FMath::Sin(Rad);

    FVector2D UV((X * C - Y * SN) / ScaleCm, (X * SN + Y * C) / ScaleCm);

    const float WarpStrength = FMath::Clamp(S.PlateauMaterialUVWarpStrength, 0.f, 0.5f);
    if (WarpStrength > 0.f)
    {
        const float WarpScale = FMath::Max(500.f, S.PlateauMaterialUVWarpScaleCm);
        const FVector2D P(X / WarpScale, Y / WarpScale);
        const float W0 = MG_PlateauFBM2D(P + FVector2D(8.1f, 2.3f), 3);
        const float W1 = MG_PlateauFBM2D(P + FVector2D(-4.7f, 15.9f), 3);
        UV += FVector2D(W0, W1) * WarpStrength;
    }

    return UV;
}

static void MG_AddPlateauQuad(
    FChunkMeshData& M,
    const FVector& A,
    const FVector& B,
    const FVector& C,
    const FVector& D,
    const FVector& DesiredNormal,
    const FVector2D& UVA = FVector2D(0.f, 0.f),
    const FVector2D& UVB = FVector2D(1.f, 0.f),
    const FVector2D& UVC = FVector2D(1.f, 1.f),
    const FVector2D& UVD = FVector2D(0.f, 1.f))
{
    const int32 Base = M.Vertices.Num();

    M.Vertices.Add(A);
    M.Vertices.Add(B);
    M.Vertices.Add(C);
    M.Vertices.Add(D);

    FVector N = DesiredNormal.GetSafeNormal();
    if (N.IsNearlyZero())
    {
        N = FVector::UpVector;
    }

    M.Normals.Add(N);
    M.Normals.Add(N);
    M.Normals.Add(N);
    M.Normals.Add(N);

    M.UV0.Add(UVA);
    M.UV0.Add(UVB);
    M.UV0.Add(UVC);
    M.UV0.Add(UVD);

    FVector TangentX = FVector::CrossProduct(FVector::UpVector, N);
    if (TangentX.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        TangentX = FVector::CrossProduct(FVector::RightVector, N);
    }
    if (!TangentX.Normalize())
    {
        TangentX = FVector::ForwardVector;
    }

    const FProcMeshTangent Tangent(TangentX, false);
    M.Tangents.Add(Tangent);
    M.Tangents.Add(Tangent);
    M.Tangents.Add(Tangent);
    M.Tangents.Add(Tangent);

    const FVector FaceN = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
    const bool bFaceNMatchesDesiredNormal = FVector::DotProduct(FaceN, N) >= 0.f;

    if (bFaceNMatchesDesiredNormal)
    {
        // Reversed winding: A-C-B / A-D-C
        M.Triangles.Add(Base + 0);
        M.Triangles.Add(Base + 2);
        M.Triangles.Add(Base + 1);

        M.Triangles.Add(Base + 0);
        M.Triangles.Add(Base + 3);
        M.Triangles.Add(Base + 2);
    }
    else
    {
        // Default winding: A-B-C / A-C-D
        M.Triangles.Add(Base + 0);
        M.Triangles.Add(Base + 1);
        M.Triangles.Add(Base + 2);

        M.Triangles.Add(Base + 0);
        M.Triangles.Add(Base + 2);
        M.Triangles.Add(Base + 3);
    }
}

static void MG_BuildTopPlateauMeshData(
    const FChunkMeshData& CliffMesh,
    const FMountainGenSettings& S,
    FChunkMeshData& Out)
{
    Out = FChunkMeshData();

    if (!S.bAddTopFlatPlateau)
    {
        return;
    }

    const float Depth = FMath::Max(0.f, S.TopPlateauDepthCm);
    const float Thickness = FMath::Max(10.f, S.TopPlateauThicknessCm);
    const float HalfW = FMath::Max(10.f, S.CliffHalfWidthCm);
    if (Depth <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const int32 SegX = FMath::Clamp(S.PlateauSegmentsX, 1, 256);
    const int32 SegY = FMath::Clamp(S.TopPlateauConformSegmentsY, 4, 512);
    const int32 PlateauSeed = FMath::Max(1, S.Seed + S.PlateauSeedOffset);

    const float XBack = -Depth;
    const float DefaultContactX = FMath::Max(0.f, S.TopPlateauFrontOverlapCm);
    const float YLeft = -HalfW;
    const float YRight = HalfW;
    const float ZBaseTop = S.BaseHeightCm + S.CliffHeightCm + S.TopPlateauHeightOffsetCm;

    TArray<FMGCliffRimSample> CliffRim;
    if (S.bConformTopPlateauToCliff)
    {
        MG_BuildCliffRimSamples(CliffMesh, S, ZBaseTop, YLeft, YRight, SegY, CliffRim);
    }
    else
    {
        CliffRim.SetNum(SegY + 1);
    }

    TArray<float> ContactX;
    TArray<float> ContactZ;
    ContactX.SetNum(SegY + 1);
    ContactZ.SetNum(SegY + 1);

    for (int32 iy = 0; iy <= SegY; ++iy)
    {
        if (CliffRim.IsValidIndex(iy) && CliffRim[iy].bValid)
        {
            ContactX[iy] = FMath::Max(CliffRim[iy].X, XBack + 10.f);
            ContactZ[iy] = CliffRim[iy].Z;
        }
        else
        {
            ContactX[iy] = DefaultContactX;
            ContactZ[iy] = ZBaseTop;
        }
    }

    auto TopPoint = [&](int32 ix, int32 iy) -> FVector
        {
            const float AX = (float)ix / (float)SegX;
            const float AY = (float)iy / (float)SegY;

            const float Y = FMath::Lerp(YLeft, YRight, AY);
            const float XFront = ContactX[iy];
            const float X = FMath::Lerp(XBack, XFront, AX);

            const float EdgeFade = MG_PlateauEdgeFade(X, Y, XBack, XFront, YLeft, YRight, S);
            const float Uplift = MG_PlateauUpliftNoiseCm(S, PlateauSeed, X, Y) * EdgeFade;

            const float NaturalTopZ = ZBaseTop + Uplift;

            // 절벽과 맞닿는 앞쪽만 절벽 상단 높이에 부드럽게 맞춘다.
            // 이 값은 윗면 접합용이다. 아랫면 높이로 직접 쓰면 Thickness와 무관한 큰 벽이 생긴다.
            const float RawContactTopZ = ContactZ[iy] + FMath::Max(0.f, S.TopPlateauContactOverlapDownCm);

            // RimStitch 방식은 유지하되, Plateau 윗면은 시작 높이 아래로 내려가지 않는다.
            // 즉, 절벽 림 X는 따라가지만 낮은 림 Z가 Plateau를 아래로 끌어내리지 못하게 한다.
            const float ContactTopZ = FMath::Max(ZBaseTop, RawContactTopZ);
            const float FrontConformAlpha = MG_PlateauSmooth01((AX - 0.72f) / 0.28f);

            const float BlendedZ = FMath::Lerp(NaturalTopZ, ContactTopZ, FrontConformAlpha);
            const float FinalZ = FMath::Max(ZBaseTop, BlendedZ);

            return FVector(X, Y, FinalZ);
        };

    auto BottomPoint = [&](int32 ix, int32 iy) -> FVector
        {
            const FVector T = TopPoint(ix, iy);

            // 아랫면은 항상 윗면 기준 Thickness만큼만 내려간다.
            // 따라서 TopPlateauThicknessCm을 줄이면 실제 보이는 두께도 같이 줄어든다.
            return FVector(T.X, T.Y, T.Z - Thickness);
        };

    Out.Vertices.Reserve((SegX + 1) * (SegY + 1) * 2 + SegX * 4 + SegY * 4);
    Out.Triangles.Reserve(SegX * SegY * 12 + SegX * 12 + SegY * 12);
    Out.Normals.Reserve((SegX + 1) * (SegY + 1) * 2 + SegX * 4 + SegY * 4);
    Out.UV0.Reserve((SegX + 1) * (SegY + 1) * 2 + SegX * 4 + SegY * 4);
    Out.Tangents.Reserve((SegX + 1) * (SegY + 1) * 2 + SegX * 4 + SegY * 4);

    // 윗면: 하나의 연속된 Heightfield처럼 생성한다. 내부 수직면은 만들지 않는다.
    for (int32 ix = 0; ix < SegX; ++ix)
    {
        for (int32 iy = 0; iy < SegY; ++iy)
        {
            const FVector A = TopPoint(ix, iy);
            const FVector B = TopPoint(ix + 1, iy);
            const FVector C = TopPoint(ix + 1, iy + 1);
            const FVector D = TopPoint(ix, iy + 1);

            MG_AddPlateauQuad(
                Out,
                A, B, C, D,
                FVector::UpVector,
                MG_MakePlateauMaterialUV(S, A.X, A.Y),
                MG_MakePlateauMaterialUV(S, B.X, B.Y),
                MG_MakePlateauMaterialUV(S, C.X, C.Y),
                MG_MakePlateauMaterialUV(S, D.X, D.Y));
        }
    }

    // 하부: 윗면에서 Thickness만큼만 내려간 얇은 닫힌 바닥.
    // 접합부 ContactZ를 직접 따라가지 않으므로 두꺼운 검은 벽이 생기지 않는다.
    for (int32 ix = 0; ix < SegX; ++ix)
    {
        for (int32 iy = 0; iy < SegY; ++iy)
        {
            const FVector A = BottomPoint(ix, iy);
            const FVector B = BottomPoint(ix, iy + 1);
            const FVector C = BottomPoint(ix + 1, iy + 1);
            const FVector D = BottomPoint(ix + 1, iy);
            MG_AddPlateauQuad(Out, A, B, C, D, -FVector::UpVector);
        }
    }

    // 앞쪽 접합면: 절벽 실루엣을 따라가는 외곽 면.
    for (int32 iy = 0; iy < SegY; ++iy)
    {
        const FVector A = BottomPoint(SegX, iy);
        const FVector B = BottomPoint(SegX, iy + 1);
        const FVector C = TopPoint(SegX, iy + 1);
        const FVector D = TopPoint(SegX, iy);
        const FVector N = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
        MG_AddPlateauQuad(Out, A, B, C, D, N.IsNearlyZero() ? FVector::ForwardVector : N);
    }

    // 뒤쪽 면.
    for (int32 iy = 0; iy < SegY; ++iy)
    {
        const FVector A = BottomPoint(0, iy + 1);
        const FVector B = BottomPoint(0, iy);
        const FVector C = TopPoint(0, iy);
        const FVector D = TopPoint(0, iy + 1);
        MG_AddPlateauQuad(Out, A, B, C, D, -FVector::ForwardVector);
    }

    // 좌우 측면.
    for (int32 ix = 0; ix < SegX; ++ix)
    {
        {
            const FVector A = BottomPoint(ix, 0);
            const FVector B = BottomPoint(ix + 1, 0);
            const FVector C = TopPoint(ix + 1, 0);
            const FVector D = TopPoint(ix, 0);
            MG_AddPlateauQuad(Out, A, B, C, D, -FVector::RightVector);
        }

        {
            const FVector A = BottomPoint(ix + 1, SegY);
            const FVector B = BottomPoint(ix, SegY);
            const FVector C = TopPoint(ix, SegY);
            const FVector D = TopPoint(ix + 1, SegY);
            MG_AddPlateauQuad(Out, A, B, C, D, FVector::RightVector);
        }
    }
}


// ============================================================
// Remove bad triangles
// ============================================================

static int32 MG_RemoveBadTriangles(FChunkMeshData& M, float MinAreaCm2, float MinEdgeCm)
{
    const int32 V = M.Vertices.Num();
    const int32 I = M.Triangles.Num();

    if (V <= 0 || I < 3)
    {
        return 0;
    }

    const int32 OriginalTriCount = I / 3;

    // 주의:
    // Marching Cubes 표면에는 매우 얇지만 실제 표면을 연결하는 삼각형이 생길 수 있다.
    // 이전 버전처럼 VoxelSize 기준의 넓은 면적/변 길이로 삼각형을 제거하면
    // 실제로 필요한 얇은 연결 삼각형까지 삭제되어 선택 시 노란 경계선처럼 보이는
    // 미세 균열/구멍이 생길 수 있다.
    // 따라서 여기서는 "보기 싫은 얇은 삼각형"을 제거하지 않고,
    // 렌더링과 인덱스 안정성을 해치는 진짜 퇴화 삼각형만 제거한다.
    MinAreaCm2 = FMath::Max(0.000001f, MinAreaCm2);
    MinEdgeCm = FMath::Max(0.0001f, MinEdgeCm);

    const float MinEdge2 = MinEdgeCm * MinEdgeCm;

    TArray<int32> NewTris;
    NewTris.Reserve(I);

    const int32 NumTris = I / 3;

    for (int32 t = 0; t < NumTris; ++t)
    {
        const int32 IA = M.Triangles[t * 3 + 0];
        const int32 IB = M.Triangles[t * 3 + 1];
        const int32 IC = M.Triangles[t * 3 + 2];

        if (!M.Vertices.IsValidIndex(IA) ||
            !M.Vertices.IsValidIndex(IB) ||
            !M.Vertices.IsValidIndex(IC))
        {
            continue;
        }

        if (IA == IB || IB == IC || IC == IA)
        {
            continue;
        }

        const FVector A = M.Vertices[IA];
        const FVector B = M.Vertices[IB];
        const FVector C = M.Vertices[IC];

        const FVector AB = B - A;
        const FVector BC = C - B;
        const FVector CA = A - C;

        const float AB2 = AB.SizeSquared();
        const float BC2 = BC.SizeSquared();
        const float CA2 = CA.SizeSquared();

        // 완전히 겹친 정점/거의 0 길이 변만 제거한다.
        // 얇지만 정상적인 표면 연결 삼각형은 유지해야 미세 균열이 생기지 않는다.
        if (AB2 < MinEdge2 || BC2 < MinEdge2 || CA2 < MinEdge2)
        {
            continue;
        }

        const FVector Cross = FVector::CrossProduct(AB, C - A);
        const float Area2 = Cross.Size(); // 실제 삼각형 면적의 2배

        if (Area2 < MinAreaCm2 * 2.0f)
        {
            continue;
        }

        NewTris.Add(IA);
        NewTris.Add(IB);
        NewTris.Add(IC);
    }

    const int32 NewTriCount = NewTris.Num() / 3;
    M.Triangles = MoveTemp(NewTris);
    return FMath::Max(0, OriginalTriCount - NewTriCount);
}

static EMGSurfaceType MG_ClassifySurfaceType(const FVector& Normal, float SlopeAngleDeg)
{
    if (Normal.Z < -0.20f)
    {
        return EMGSurfaceType::Overhang;
    }

    if (SlopeAngleDeg >= 78.f)
    {
        return EMGSurfaceType::Wall;
    }

    if (SlopeAngleDeg >= 58.f)
    {
        return EMGSurfaceType::Cliff;
    }

    if (SlopeAngleDeg >= 42.f)
    {
        return EMGSurfaceType::Steep;
    }

    return EMGSurfaceType::Ground;
}

static float MG_ComputeDangerScore(const EMGSurfaceType SurfaceType, float SlopeAngleDeg)
{
    float Danger = FMath::Clamp(SlopeAngleDeg / 90.f, 0.f, 1.f);

    if (SurfaceType == EMGSurfaceType::Overhang)
    {
        Danger = FMath::Max(Danger, 0.85f);
    }
    else if (SurfaceType == EMGSurfaceType::Wall || SurfaceType == EMGSurfaceType::Cliff)
    {
        Danger = FMath::Max(Danger, 0.65f);
    }

    return Danger;
}

static void MG_AnalyzeMeshQuality(
    const FChunkMeshData& MeshData,
    int32 RemovedBadTriangles,
    float MinThinAreaCm2,
    FMGMeshQualityReport& OutReport)
{
    OutReport = FMGMeshQualityReport();

    const int32 V = MeshData.Vertices.Num();
    const int32 T = MeshData.Triangles.Num() / 3;

    OutReport.VertexCount = V;
    OutReport.TriangleCount = T;
    OutReport.RemovedBadTriangleCount = RemovedBadTriangles;
    OutReport.BadTriangleRatio = (T + RemovedBadTriangles) > 0
        ? (float)RemovedBadTriangles / (float)(T + RemovedBadTriangles)
        : 0.f;

    if (V <= 0 || T <= 0)
    {
        return;
    }

    int32 ThinCount = 0;
    int32 NormalRiskCount = 0;

    for (int32 t = 0; t < T; ++t)
    {
        const int32 IA = MeshData.Triangles[t * 3 + 0];
        const int32 IB = MeshData.Triangles[t * 3 + 1];
        const int32 IC = MeshData.Triangles[t * 3 + 2];

        if (!MeshData.Vertices.IsValidIndex(IA) || !MeshData.Vertices.IsValidIndex(IB) || !MeshData.Vertices.IsValidIndex(IC))
        {
            continue;
        }

        const FVector A = MeshData.Vertices[IA];
        const FVector B = MeshData.Vertices[IB];
        const FVector C = MeshData.Vertices[IC];

        const FVector FaceCross = FVector::CrossProduct(B - A, C - A);
        const float Area = FaceCross.Size() * 0.5f;

        if (Area < MinThinAreaCm2)
        {
            ++ThinCount;
        }

        FVector FaceNormal = FaceCross.GetSafeNormal();
        FVector AvgNormal = FVector::ZeroVector;
        if (MeshData.Normals.IsValidIndex(IA)) AvgNormal += MeshData.Normals[IA];
        if (MeshData.Normals.IsValidIndex(IB)) AvgNormal += MeshData.Normals[IB];
        if (MeshData.Normals.IsValidIndex(IC)) AvgNormal += MeshData.Normals[IC];

        if (FaceNormal.IsNearlyZero() || !AvgNormal.Normalize() || FVector::DotProduct(FaceNormal, AvgNormal) < 0.35f)
        {
            ++NormalRiskCount;
        }
    }

    OutReport.ThinTriangleRatio = T > 0 ? (float)ThinCount / (float)T : 0.f;
    OutReport.NormalRiskRatio = T > 0 ? (float)NormalRiskCount / (float)T : 0.f;
}

static void MG_BuildSurfaceSamples(
    const FChunkMeshData& MeshData,
    const FVector& ActorWorld,
    int32 TriangleStride,
    float MonsterMaxSlopeDeg,
    float ItemMaxSlopeDeg,
    float PlatformMaxSlopeDeg,
    TArray<FMGSurfaceSample>& OutSamples)
{
    OutSamples.Reset();

    const int32 T = MeshData.Triangles.Num() / 3;
    if (T <= 0)
    {
        return;
    }

    TriangleStride = FMath::Clamp(TriangleStride, 1, 256);
    OutSamples.Reserve(FMath::Max(1, T / TriangleStride));

    for (int32 t = 0; t < T; t += TriangleStride)
    {
        const int32 IA = MeshData.Triangles[t * 3 + 0];
        const int32 IB = MeshData.Triangles[t * 3 + 1];
        const int32 IC = MeshData.Triangles[t * 3 + 2];

        if (!MeshData.Vertices.IsValidIndex(IA) || !MeshData.Vertices.IsValidIndex(IB) || !MeshData.Vertices.IsValidIndex(IC))
        {
            continue;
        }

        const FVector A = MeshData.Vertices[IA];
        const FVector B = MeshData.Vertices[IB];
        const FVector C = MeshData.Vertices[IC];

        FVector N = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
        if (N.IsNearlyZero())
        {
            continue;
        }

        if (MeshData.Normals.IsValidIndex(IA) && MeshData.Normals.IsValidIndex(IB) && MeshData.Normals.IsValidIndex(IC))
        {
            const FVector AvgN = (MeshData.Normals[IA] + MeshData.Normals[IB] + MeshData.Normals[IC]).GetSafeNormal();
            if (!AvgN.IsNearlyZero())
            {
                N = AvgN;
            }
        }

        const float SlopeDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(N.Z, -1.f, 1.f)));
        const EMGSurfaceType SurfaceType = MG_ClassifySurfaceType(N, SlopeDeg);

        FMGSurfaceSample Sample;
        Sample.Location = ActorWorld + ((A + B + C) / 3.f);
        Sample.Normal = N;
        Sample.SlopeAngleDeg = SlopeDeg;
        Sample.SurfaceType = SurfaceType;
        Sample.DangerScore = MG_ComputeDangerScore(SurfaceType, SlopeDeg);

        const bool bBlocked = (SurfaceType == EMGSurfaceType::Blocked);
        const bool bOverhang = (SurfaceType == EMGSurfaceType::Overhang);
        const bool bWall = (SurfaceType == EMGSurfaceType::Wall);

        Sample.bCanPlaceMonster = !bBlocked && !bOverhang && !bWall && SlopeDeg <= MonsterMaxSlopeDeg;
        Sample.bCanPlaceItem = !bBlocked && !bOverhang && SlopeDeg <= ItemMaxSlopeDeg;
        Sample.bCanPlacePlatform = !bBlocked && SlopeDeg <= PlatformMaxSlopeDeg;

        Sample.Usage.bGameplay = (SurfaceType == EMGSurfaceType::Cliff || SurfaceType == EMGSurfaceType::Wall || SurfaceType == EMGSurfaceType::Overhang);
        Sample.Usage.bPlacement = Sample.bCanPlaceMonster || Sample.bCanPlaceItem || Sample.bCanPlacePlatform;

        OutSamples.Add(Sample);
    }
}

static void MG_BuildZoneReports(
    const TArray<FMGSurfaceSample>& Samples,
    const TArray<FMGGenerationZone>& Zones,
    const FVector& ActorWorld,
    TArray<FMGZoneMetricReport>& OutReports)
{
    OutReports.Reset();

    for (const FMGGenerationZone& Zone : Zones)
    {
        if (!Zone.bEnabled)
        {
            continue;
        }

        const float MinZ = ActorWorld.Z + Zone.RelativeZMinCm;
        const float MaxZ = ActorWorld.Z + Zone.RelativeZMaxCm;
        if (MaxZ <= MinZ)
        {
            continue;
        }

        FMGZoneMetricReport R;
        R.ZoneName = Zone.ZoneName.IsNone() ? FName(TEXT("Zone")) : Zone.ZoneName;

        int32 OverhangCount = 0;
        int32 SteepCount = 0;
        int32 PlaceCount = 0;

        for (const FMGSurfaceSample& S : Samples)
        {
            if (S.Location.Z < MinZ || S.Location.Z > MaxZ)
            {
                continue;
            }

            ++R.SampleCount;
            if (S.SurfaceType == EMGSurfaceType::Overhang) ++OverhangCount;
            if (S.SurfaceType == EMGSurfaceType::Steep || S.SurfaceType == EMGSurfaceType::Cliff || S.SurfaceType == EMGSurfaceType::Wall) ++SteepCount;
            if (S.Usage.bPlacement) ++PlaceCount;
        }

        if (R.SampleCount > 0)
        {
            R.OverhangRatio = (float)OverhangCount / (float)R.SampleCount;
            R.SteepRatio = (float)SteepCount / (float)R.SampleCount;
            R.PlacementRatio = (float)PlaceCount / (float)R.SampleCount;
        }

        OutReports.Add(R);
    }
}

static void MG_FillReportMetrics(const FMountainGenSettings& S, const FMGMetrics& M, FMGGenerationReport& OutReport)
{
    OutReport.Metrics.Reset();

    auto AddMetric = [&OutReport](FName Name, float Value, float Score, bool bPassed)
        {
            FGDPCGMetricValue V;
            V.MetricName = Name;
            V.Value = Value;
            V.Score = Score;
            V.bPassed = bPassed;
            OutReport.Metrics.Add(V);
        };

    const bool bOverOK = (M.OverhangRatio >= S.Targets.OverhangMin && M.OverhangRatio <= S.Targets.OverhangMax);
    const bool bSteepOK = (M.SteepRatio >= S.Targets.SteepMin && M.SteepRatio <= S.Targets.SteepMax);
    const bool bRoughOK = (M.RoughnessRatio <= S.Targets.RoughnessMax);
    const bool bShadowOK = (M.ShadowRiskRatio <= S.Targets.ShadowRiskMax);

    const float OverScore = bOverOK ? 0.f : FMath::Min(FMath::Abs(M.OverhangRatio - S.Targets.OverhangMin), FMath::Abs(M.OverhangRatio - S.Targets.OverhangMax));
    const float SteepScore = bSteepOK ? 0.f : FMath::Min(FMath::Abs(M.SteepRatio - S.Targets.SteepMin), FMath::Abs(M.SteepRatio - S.Targets.SteepMax));
    const float RoughScore = FMath::Max(0.f, M.RoughnessRatio - S.Targets.RoughnessMax);
    const float ShadowScore = FMath::Max(0.f, M.ShadowRiskRatio - S.Targets.ShadowRiskMax);

    AddMetric(TEXT("OverhangRatio"), M.OverhangRatio, OverScore, bOverOK);
    AddMetric(TEXT("SteepRatio"), M.SteepRatio, SteepScore, bSteepOK);
    AddMetric(TEXT("RoughnessRatio"), M.RoughnessRatio, RoughScore, bRoughOK);
    AddMetric(TEXT("ShadowRiskRatio"), M.ShadowRiskRatio, ShadowScore, bShadowOK);

    OutReport.FinalScore = OverScore + SteepScore + RoughScore + ShadowScore;
    OutReport.bPassedAllTargets = bOverOK && bSteepOK && bRoughOK && bShadowOK;
}

// ============================================================
// Recompute normals / tangents after mesh topology changes
// ============================================================

static void MG_RecomputeNormalsAndTangents(FChunkMeshData& M)
{
    const int32 V = M.Vertices.Num();
    const int32 I = M.Triangles.Num();

    if (V <= 0 || I < 3)
    {
        return;
    }

    const TArray<FVector> OldNormals = M.Normals;

    M.Normals.SetNumZeroed(V);
    M.Tangents.SetNumZeroed(V);

    const int32 NumTris = I / 3;

    for (int32 t = 0; t < NumTris; ++t)
    {
        const int32 IA = M.Triangles[t * 3 + 0];
        const int32 IB = M.Triangles[t * 3 + 1];
        const int32 IC = M.Triangles[t * 3 + 2];

        if (!M.Vertices.IsValidIndex(IA) ||
            !M.Vertices.IsValidIndex(IB) ||
            !M.Vertices.IsValidIndex(IC))
        {
            continue;
        }

        const FVector A = M.Vertices[IA];
        const FVector B = M.Vertices[IB];
        const FVector C = M.Vertices[IC];

        const FVector AB = B - A;
        const FVector AC = C - A;

        FVector FaceNormal = FVector::CrossProduct(AB, AC);

        if (!FaceNormal.Normalize())
        {
            continue;
        }

        FVector ReferenceNormal = FVector::ZeroVector;

        if (OldNormals.IsValidIndex(IA)) ReferenceNormal += OldNormals[IA];
        if (OldNormals.IsValidIndex(IB)) ReferenceNormal += OldNormals[IB];
        if (OldNormals.IsValidIndex(IC)) ReferenceNormal += OldNormals[IC];

        if (ReferenceNormal.Normalize())
        {
            if (FVector::DotProduct(FaceNormal, ReferenceNormal) < 0.f)
            {
                FaceNormal *= -1.f;
            }
        }

        M.Normals[IA] += FaceNormal;
        M.Normals[IB] += FaceNormal;
        M.Normals[IC] += FaceNormal;
    }

    for (int32 i = 0; i < V; ++i)
    {
        if (!M.Normals[i].Normalize())
        {
            if (OldNormals.IsValidIndex(i) && OldNormals[i].SizeSquared() > KINDA_SMALL_NUMBER)
            {
                M.Normals[i] = OldNormals[i].GetSafeNormal();
            }
            else
            {
                M.Normals[i] = FVector::UpVector;
            }
        }

        // 절벽 수직면에서도 안정적으로 동작하는 탄젠트 생성.
        // UpVector와 거의 평행하면 RightVector를 보조축으로 사용한다.
        FVector TangentX = FVector::CrossProduct(FVector::UpVector, M.Normals[i]);

        if (TangentX.SizeSquared() < KINDA_SMALL_NUMBER)
        {
            TangentX = FVector::CrossProduct(FVector::RightVector, M.Normals[i]);
        }

        if (!TangentX.Normalize())
        {
            TangentX = FVector::ForwardVector;
        }

        M.Tangents[i] = FProcMeshTangent(TangentX, false);
    }
}

// ============================================================
// Actor
// ============================================================

AMountainGenWorldActor::AMountainGenWorldActor()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->bUseAsyncCooking = true;
    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->SetGenerateOverlapEvents(false);
    ProcMesh->SetMobility(EComponentMobility::Static);

    TopPlateauMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TopPlateauMesh"));
    TopPlateauMesh->SetupAttachment(ProcMesh);
    TopPlateauMesh->bUseAsyncCooking = true;
    TopPlateauMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    TopPlateauMesh->SetGenerateOverlapEvents(false);
    TopPlateauMesh->SetMobility(EComponentMobility::Static);
}

void AMountainGenWorldActor::UI_Status(const FString& Msg, float Seconds, FColor Color) const
{
    if (!bEnableOnScreenMessages)
    {
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, Seconds, Color, Msg);
    }
}

void AMountainGenWorldActor::ToggleOnScreenMessages()
{
    bEnableOnScreenMessages = !bEnableOnScreenMessages;

    if (GEngine)
    {
        const FString Msg = bEnableOnScreenMessages
            ? TEXT("[MountainGen] 화면 출력: ON")
            : TEXT("[MountainGen] 화면 출력: OFF");

        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, Msg);
    }
}

void AMountainGenWorldActor::SetOnScreenMessagesEnabled(bool bEnabled)
{
    bEnableOnScreenMessages = bEnabled;

    if (GEngine)
    {
        const FString Msg = bEnableOnScreenMessages
            ? TEXT("[MountainGen] 화면 출력: ON")
            : TEXT("[MountainGen] 화면 출력: OFF");

        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, Msg);
    }
}

void AMountainGenWorldActor::ApplyVoxelMaterialParameters()
{
    if (!ProcMesh || !VoxelMaterial)
        return;

    if (!VoxelMID || VoxelMID->Parent != VoxelMaterial)
    {
        VoxelMID = UMaterialInstanceDynamic::Create(VoxelMaterial, this);
    }

    if (!VoxelMID)
    {
        ProcMesh->SetMaterial(0, VoxelMaterial);
        return;
    }

    VoxelMID->SetScalarParameterValue(TEXT("SnowSlopeMinZ"), SnowSlopeMinZ);
    VoxelMID->SetScalarParameterValue(TEXT("SnowSlopeMaxZ"), SnowSlopeMaxZ);
    VoxelMID->SetScalarParameterValue(TEXT("OverhangMaxZ"), OverhangMaxZ);
    VoxelMID->SetScalarParameterValue(TEXT("SnowNoiseScale"), SnowNoiseScale);
    VoxelMID->SetScalarParameterValue(TEXT("SnowNoiseStrength"), SnowNoiseStrength);

    ProcMesh->SetMaterial(0, VoxelMID);
}

static bool MG_InRange(float V, float Min, float Max)
{
    return (V >= Min && V <= Max);
}

FString AMountainGenWorldActor::MakeMetricsLine(
    const FMountainGenSettings& S,
    const FMGMetrics& M,
    bool& bOutOverhangOK,
    bool& bOutSteepOK)
{
    bOutOverhangOK = MG_InRange(M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax);
    bOutSteepOK = MG_InRange(M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax);

    const bool bRoughOK = (M.RoughnessRatio <= S.Targets.RoughnessMax);
    const bool bShadowOK = (M.ShadowRiskRatio <= S.Targets.ShadowRiskMax);

    return FString::Printf(
        TEXT("Overhang %.3f [%.3f~%.3f] %s | Steep %.3f [%.3f~%.3f] %s | Rough %.3f <= %.3f %s | Shadow %.3f <= %.3f %s | Near=%d"),
        M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax, bOutOverhangOK ? TEXT("OK") : TEXT("FAIL"),
        M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax, bOutSteepOK ? TEXT("OK") : TEXT("FAIL"),
        M.RoughnessRatio, S.Targets.RoughnessMax, bRoughOK ? TEXT("OK") : TEXT("FAIL"),
        M.ShadowRiskRatio, S.Targets.ShadowRiskMax, bShadowOK ? TEXT("OK") : TEXT("FAIL"),
        M.SurfaceNearSamples
    );
}

#if WITH_EDITOR
static uint32 MG_HashBoolForSettings(bool bValue)
{
    return ::GetTypeHash(bValue ? 1 : 0);
}

static uint32 MG_HashFloatForSettings(float Value)
{
    return ::GetTypeHash(FMath::RoundToInt(Value * 1000.0f));
}

static uint32 MG_HashNameForSettings(FName Value)
{
    const FString S = Value.ToString();
    return FCrc::StrCrc32(*S);
}

uint32 AMountainGenWorldActor::ComputeSettingsHash_Editor() const
{
    uint32 H = 0;

    // 재현성
    H = HashCombine(H, ::GetTypeHash(Settings.Seed));

    // 난이도 / AutoTune
    H = HashCombine(H, ::GetTypeHash((uint8)Settings.Difficulty));
    H = HashCombine(H, MG_HashBoolForSettings(Settings.bAutoTune));
    H = HashCombine(H, ::GetTypeHash(Settings.FeedbackIters));
    H = HashCombine(H, ::GetTypeHash(Settings.SeedSearchTries));
    H = HashCombine(H, MG_HashBoolForSettings(Settings.bRetrySeedUntilSatisfied));
    H = HashCombine(H, ::GetTypeHash(Settings.MaxSeedAttempts));

    H = HashCombine(H, MG_HashFloatForSettings(Settings.Targets.OverhangMin));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.Targets.OverhangMax));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.Targets.SteepMin));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.Targets.SteepMax));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.Targets.RoughnessMax));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.Targets.ShadowRiskMax));

    // Voxel / Mesh
    H = HashCombine(H, MG_HashFloatForSettings(Settings.VoxelSizeCm));
    H = HashCombine(H, ::GetTypeHash((uint8)Settings.TerrainAlgorithm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.IsoLevel));
    H = HashCombine(H, MG_HashBoolForSettings(Settings.bCreateCollision));

    // 기본 위치 / 절벽 크기
    H = HashCombine(H, MG_HashFloatForSettings(Settings.BaseHeightCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.CliffHalfWidthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.CliffHeightCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.CliffThicknessCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.CliffDepthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.FrontBandDepthCm));

    // 상단 평지 후처리
    H = HashCombine(H, MG_HashBoolForSettings(Settings.bAddTopFlatPlateau));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauDepthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauThicknessCm));
    H = HashCombine(H, MG_HashBoolForSettings(Settings.bConformTopPlateauToCliff));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauFrontOverlapCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauHeightOffsetCm));
    H = HashCombine(H, ::GetTypeHash(Settings.TopPlateauConformSegmentsY));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauConformSampleBandCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauConformSearchDepthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauContactOverlapDownCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.TopPlateauMaxConformDropCm));
    H = HashCombine(H, ::GetTypeHash(Settings.PlateauSeedOffset));
    H = HashCombine(H, ::GetTypeHash(Settings.PlateauSegmentsX));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauSurfaceNoiseStrengthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauSurfaceNoiseScaleCm));
    H = HashCombine(H, ::GetTypeHash(Settings.PlateauSurfaceNoiseOctaves));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauSurfaceEdgeFadeCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauMaterialUVScaleCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauMaterialUVRotationDeg));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauMaterialUVWarpStrength));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.PlateauMaterialUVWarpScaleCm));
    H = HashCombine(H, MG_HashBoolForSettings(bTopPlateauCreateCollision));

    // 밀도장 / 디테일
    H = HashCombine(H, MG_HashFloatForSettings(Settings.BaseField3DStrengthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.BaseField3DScaleCm));
    H = HashCombine(H, ::GetTypeHash(Settings.BaseField3DOctaves));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.DetailScaleCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.DetailStrengthCm));
    H = HashCombine(H, ::GetTypeHash(Settings.DetailOctaves));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.SurfaceRoughnessStrengthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.SurfaceRoughnessMaskStrength));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.SurfaceQualityScoreWeight));

    // 오버행 / 언더컷
    H = HashCombine(H, MG_HashFloatForSettings(Settings.VolumeStrength));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.OverhangScaleCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.OverhangBias));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.OverhangDepthCm));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.OverhangFadeCm));

    // Metrics
    H = HashCombine(H, MG_HashFloatForSettings(Settings.MetricsStepCm));
    H = HashCombine(H, ::GetTypeHash(Settings.MetricsSamplesPerTry));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.SteepDotOverride));

    // 메시 후처리
    H = HashCombine(H, MG_HashBoolForSettings(bEnablePostWeld));
    H = HashCombine(H, MG_HashFloatForSettings(PostWeldEpsilonScale));
    H = HashCombine(H, MG_HashBoolForSettings(bRepairMeshSeams));
    H = HashCombine(H, MG_HashFloatForSettings(MeshSeamWeldEpsilonScale));
    H = HashCombine(H, MG_HashBoolForSettings(bEnableIslandCull));
    H = HashCombine(H, ::GetTypeHash(MinTrisToKeepAfterCull));

    // Goal-driven hierarchical search
    H = HashCombine(H, MG_HashBoolForSettings(Settings.bUseHierarchicalGoalSearch));
    H = HashCombine(H, ::GetTypeHash(Settings.ProxySeedBudget));
    H = HashCombine(H, ::GetTypeHash(Settings.ProxySurvivorCount));
    H = HashCombine(H, ::GetTypeHash(Settings.ProxyMetricsSamplesPerTry));
    H = HashCombine(H, ::GetTypeHash(Settings.GoalFeedbackRounds));
    H = HashCombine(H, ::GetTypeHash(Settings.GoalFeedbackBatchSize));
    H = HashCombine(H, MG_HashFloatForSettings(Settings.HardConstraintPenaltyWeight));

    // Surface/query/report settings
    H = HashCombine(H, ::GetTypeHash(SurfaceSampleTriangleStride));
    H = HashCombine(H, MG_HashFloatForSettings(MonsterMaxSlopeDeg));
    H = HashCombine(H, MG_HashFloatForSettings(ItemMaxSlopeDeg));
    H = HashCombine(H, MG_HashFloatForSettings(PlatformMaxSlopeDeg));

    // Zone settings
    H = HashCombine(H, ::GetTypeHash(GenerationZones.Num()));

    for (const FMGGenerationZone& Zone : GenerationZones)
    {
        H = HashCombine(H, MG_HashNameForSettings(Zone.ZoneName));
        H = HashCombine(H, MG_HashBoolForSettings(Zone.bEnabled));
        H = HashCombine(H, MG_HashFloatForSettings(Zone.RelativeZMinCm));
        H = HashCombine(H, MG_HashFloatForSettings(Zone.RelativeZMaxCm));
        H = HashCombine(H, MG_HashFloatForSettings(Zone.TargetOverhangMin));
        H = HashCombine(H, MG_HashFloatForSettings(Zone.TargetOverhangMax));
        H = HashCombine(H, MG_HashFloatForSettings(Zone.TargetSteepMin));
        H = HashCombine(H, MG_HashFloatForSettings(Zone.TargetSteepMax));
    }

    return H;
}
#endif

void AMountainGenWorldActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UWorld* W = GetWorld();

    if (W && W->IsGameWorld())
        return;

#if WITH_EDITOR
    const uint32 NewHash = ComputeSettingsHash_Editor();
    const bool bFirst = (LastSettingsHash_Editor == 0);
    const bool bSettingsChanged = (!bFirst && NewHash != LastSettingsHash_Editor);

    if (bFirst || bSettingsChanged)
    {
        LastSettingsHash_Editor = NewHash;
        BuildChunkAndMesh();
    }
    return;
#else
    BuildChunkAndMesh();
#endif
}

#if WITH_EDITOR
void AMountainGenWorldActor::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    (void)bFinished;
}

void AMountainGenWorldActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (!PropertyChangedEvent.Property) return;

    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
    BuildChunkAndMesh();
}
#endif

void AMountainGenWorldActor::BeginPlay()
{
    Super::BeginPlay();

    if (ProcMesh)
    {
        const bool bHasExistingSection = (ProcMesh->GetNumSections() > 0);
        const bool bHasValidBounds = (ProcMesh->Bounds.GetBox().IsValid != 0);

        if (bHasExistingSection || bHasValidBounds)
        {
            UpdateGeneratedMeshStateAndBroadcast();
        }
        else if (!bAsyncWorking)
        {
            BuildChunkAndMesh();
        }
    }

    const bool bNeedAnyRuntimeKey =
        bEnableRandomSeedKey ||
        bEnableOnScreenToggleKey;

    if (!bNeedAnyRuntimeKey)
    {
        return;
    }

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC)
    {
        return;
    }

    EnableInput(PC);

    if (!InputComponent)
    {
        InputComponent = NewObject<UInputComponent>(this, TEXT("MGInputComponent"));
        InputComponent->RegisterComponent();
        AddInstanceComponent(InputComponent);
        PC->PushInputComponent(InputComponent);
    }

    if (bEnableRandomSeedKey)
    {
        InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AMountainGenWorldActor::RandomizeSeed);
        InputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &AMountainGenWorldActor::RandomizeSeed);

        InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AMountainGenWorldActor::CycleDifficulty);
        InputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &AMountainGenWorldActor::CycleDifficulty);

        UI_Status(TEXT("[MountainGen] 1 키: 시드 랜덤 변경"), 2.0f, FColor::Green);
    }

    if (bEnableOnScreenToggleKey)
    {
        InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AMountainGenWorldActor::ToggleOnScreenMessages);
    }
}


bool AMountainGenWorldActor::QuerySurfaceAtLocation(const FVector& WorldLocation, FMGSurfaceSample& OutSample, float SearchRadiusCm) const
{
    SearchRadiusCm = FMath::Max(1.f, SearchRadiusCm);
    const float MaxDist2 = SearchRadiusCm * SearchRadiusCm;

    bool bFound = false;
    float BestDist2 = MaxDist2;

    for (const FMGSurfaceSample& Sample : GeneratedSurfaceSamples)
    {
        const float D2 = FVector::DistSquared(WorldLocation, Sample.Location);
        if (D2 <= BestDist2)
        {
            BestDist2 = D2;
            OutSample = Sample;
            bFound = true;
        }
    }

    return bFound;
}

void AMountainGenWorldActor::GetPlacementCandidates(TArray<FMGSurfaceSample>& OutCandidates, EMGPlacementUsage Usage) const
{
    OutCandidates.Reset();

    for (const FMGSurfaceSample& Sample : GeneratedSurfaceSamples)
    {
        bool bAccept = false;
        switch (Usage)
        {
        case EMGPlacementUsage::Monster:
            bAccept = Sample.bCanPlaceMonster;
            break;
        case EMGPlacementUsage::Item:
            bAccept = Sample.bCanPlaceItem;
            break;
        case EMGPlacementUsage::Platform:
            bAccept = Sample.bCanPlacePlatform;
            break;
        case EMGPlacementUsage::Gameplay:
            bAccept = Sample.Usage.bGameplay;
            break;
        default:
            break;
        }

        if (bAccept)
        {
            OutCandidates.Add(Sample);
        }
    }
}

bool AMountainGenWorldActor::IsLocationValidForPlacement(const FVector& WorldLocation, EMGPlacementUsage Usage, float SearchRadiusCm) const
{
    FMGSurfaceSample Sample;
    if (!QuerySurfaceAtLocation(WorldLocation, Sample, SearchRadiusCm))
    {
        return false;
    }

    switch (Usage)
    {
    case EMGPlacementUsage::Monster:
        return Sample.bCanPlaceMonster;
    case EMGPlacementUsage::Item:
        return Sample.bCanPlaceItem;
    case EMGPlacementUsage::Platform:
        return Sample.bCanPlacePlatform;
    case EMGPlacementUsage::Gameplay:
        return Sample.Usage.bGameplay;
    default:
        return false;
    }
}

void AMountainGenWorldActor::GetGeneratedSurfaceSamples(TArray<FMGSurfaceSample>& OutSamples) const
{
    OutSamples = GeneratedSurfaceSamples;
}

void AMountainGenWorldActor::RebuildTopPlateauMesh(const FChunkMeshData& CliffMeshData, const FMountainGenSettings& FinalSettings)
{
    if (!TopPlateauMesh)
    {
        return;
    }

    TopPlateauMesh->ClearAllMeshSections();
    TopPlateauMesh->ClearCollisionConvexMeshes();

    FChunkMeshData PlateauMeshData;
    MG_BuildTopPlateauMeshData(CliffMeshData, FinalSettings, PlateauMeshData);

    MG_RecomputeNormalsAndTangents(PlateauMeshData);

    if (PlateauMeshData.Vertices.Num() == 0 || PlateauMeshData.Triangles.Num() == 0)
    {
        TopPlateauMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return;
    }

    // TopPlateau는 절벽 MeshData와 분리된 독립 컴포넌트다.
    // 따라서 목표 지표, Surface Metadata, 몬스터/아이템/플랫폼 배치 후보에는 포함하지 않는다.
    TArray<FLinearColor> PlateauColors;
    PlateauColors.Init(FLinearColor::White, PlateauMeshData.Vertices.Num());

    TopPlateauMesh->CreateMeshSection_LinearColor(
        0,
        PlateauMeshData.Vertices,
        PlateauMeshData.Triangles,
        PlateauMeshData.Normals,
        PlateauMeshData.UV0,
        PlateauColors,
        PlateauMeshData.Tangents,
        bTopPlateauCreateCollision
    );

    TopPlateauMesh->SetCollisionEnabled(
        bTopPlateauCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
    );

    if (TopPlateauMaterial)
    {
        TopPlateauMesh->SetMaterial(0, TopPlateauMaterial);
    }
}

void AMountainGenWorldActor::ApplyGeneratedMeshResult(FMGAsyncResult&& Result, bool bShowRuntimeSeedMessage)
{
    if (Result.BuildSerial != InFlightBuildSerial)
    {
        return;
    }

    if (!ProcMesh)
    {
        bAsyncWorking = false;
        InFlightBuildSerial = 0;
        return;
    }

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();
    if (TopPlateauMesh)
    {
        TopPlateauMesh->ClearAllMeshSections();
        TopPlateauMesh->ClearCollisionConvexMeshes();
    }

    if (Result.MeshData.Vertices.Num() == 0 || Result.MeshData.Triangles.Num() == 0)
    {
        UI_Status(TEXT("[MountainGen] 생성 실패: MeshData 비어있음"), 2.0f, FColor::Red);
        bAsyncWorking = false;
        InFlightBuildSerial = 0;

        if (bRegenQueued)
        {
            bRegenQueued = false;
            BuildChunkAndMesh();
        }
        return;
    }

    // Build shared surface metadata/report for runtime-generated results.
    GeneratedSurfaceSamples.Reset();
    LastGenerationReport = FMGGenerationReport();
    LastGenerationReport.FinalSeed = Result.FinalSettings.Seed;

    MG_BuildSurfaceSamples(
        Result.MeshData,
        GetActorLocation(),
        SurfaceSampleTriangleStride,
        MonsterMaxSlopeDeg,
        ItemMaxSlopeDeg,
        PlatformMaxSlopeDeg,
        GeneratedSurfaceSamples
    );

    {
        const FMountainGenSettings& FS = Result.FinalSettings;
        const float Voxel = FMath::Max(1.f, FS.VoxelSizeCm);
        const FVector ActorWorld = GetActorLocation();
        const float FrontX = FMath::Max(200.f, FS.CliffThicknessCm);
        const FVector TerrainOriginWorld = ActorWorld - FVector(FrontX, 0.f, 0.f);
        const float Band = FMath::Max(FS.CliffDepthCm, Voxel * 2.f);
        const float HalfW = FMath::Max(1.f, FS.CliffHalfWidthCm);
        const float H = FMath::Max(1.f, FS.CliffHeightCm);
        const FVector WorldMin = TerrainOriginWorld + FVector(FrontX - Band, -HalfW, FS.BaseHeightCm);
        const FVector WorldMax = TerrainOriginWorld + FVector(FrontX + Band, +HalfW, FS.BaseHeightCm + H);

        const FMGMetrics FinalMetricsForReport = MGComputeMetricsQuick(FS, TerrainOriginWorld, WorldMin, WorldMax);
        MG_FillReportMetrics(FS, FinalMetricsForReport, LastGenerationReport);
        MG_AnalyzeMeshQuality(Result.MeshData, Result.RemovedBadTriangleCount, FMath::Square(FS.VoxelSizeCm * 0.08f), LastGenerationReport.MeshQuality);
        MG_BuildZoneReports(GeneratedSurfaceSamples, GenerationZones, GetActorLocation(), LastGenerationReport.ZoneReports);
        LastGenerationReport.SurfaceSampleCount = GeneratedSurfaceSamples.Num();
    }

    ReusableColors.SetNumUninitialized(Result.MeshData.Vertices.Num());

    for (FLinearColor& C : ReusableColors)
    {
        C = FLinearColor::White;
    }

    ProcMesh->CreateMeshSection_LinearColor(
        0,
        Result.MeshData.Vertices,
        Result.MeshData.Triangles,
        Result.MeshData.Normals,
        Result.MeshData.UV0,
        ReusableColors,
        Result.MeshData.Tangents,
        Result.FinalSettings.bCreateCollision
    );

    ProcMesh->SetCollisionEnabled(
        Result.FinalSettings.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
    );

    ApplyVoxelMaterialParameters();
    RebuildTopPlateauMesh(Result.MeshData, Result.FinalSettings);

    Settings.Seed = Result.FinalSettings.Seed;

    UpdateGeneratedMeshStateAndBroadcast();

    if (bShowRuntimeSeedMessage)
    {
        UI_Status(FString::Printf(TEXT("[MountainGen] 시드 변경 완료: %d"), Settings.Seed), 2.5f, FColor::Yellow);
    }

    bAsyncWorking = false;
    InFlightBuildSerial = 0;

    if (bRegenQueued)
    {
        bRegenQueued = false;
        BuildChunkAndMesh();
    }
}

void AMountainGenWorldActor::Regenerate()
{
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::SetSeed(int32 NewSeed)
{
    const int32 Clamped = FMath::Max(1, NewSeed);

    if (bAsyncWorking)
    {
        UI_Status(TEXT("[MountainGen] 작업 중... 완료 후 다시 시도"), 1.2f, FColor::Red);
        return;
    }

    if (Settings.Seed == Clamped) return;

    Settings.Seed = Clamped;

    UI_Status(FString::Printf(TEXT("[MountainGen] 시드 변경 요청: %d"), Settings.Seed), 1.5f, FColor::Cyan);

#if WITH_EDITOR
    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
#endif

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::RandomizeSeed()
{
    if (bAsyncWorking)
    {
        UI_Status(TEXT("[MountainGen] 작업 중이라 시드 변경이 비활성화됨"), 1.2f, FColor::Orange);
        return;
    }

    UI_Status(TEXT("[MountainGen] 시드 변경(랜덤) 요청..."), 1.2f, FColor::Cyan);

    const uint64 T = FPlatformTime::Cycles64();
    const FVector L = GetActorLocation();

    uint32 Mix =
        (uint32)(T) ^
        (uint32)(T >> 32) ^
        (uint32)(PTRINT)this ^
        (uint32)FMath::RoundToInt(L.X) ^
        ((uint32)FMath::RoundToInt(L.Y) << 11) ^
        ((uint32)FMath::RoundToInt(L.Z) << 22) ^
        (uint32)(++CurrentBuildSerial * 977u);

    int32 NewSeed = (int32)(Mix & 0x7fffffff);
    if (NewSeed <= 0) NewSeed = 1;

    Settings.Seed = NewSeed;

#if WITH_EDITOR
    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
#endif

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::CycleDifficulty()
{
    using ED = EMountainGenDifficulty;

    switch (Settings.Difficulty)
    {
    case ED::Easy:   Settings.Difficulty = ED::Normal; break;
    case ED::Normal: Settings.Difficulty = ED::Hard;   break;
    case ED::Hard:   Settings.Difficulty = ED::Easy;   break;
    default:         Settings.Difficulty = ED::Easy;   break;
    }

    FString DifficultyText;
    switch (Settings.Difficulty)
    {
    case ED::Easy:   DifficultyText = TEXT("Easy"); break;
    case ED::Normal: DifficultyText = TEXT("Normal"); break;
    case ED::Hard:   DifficultyText = TEXT("Hard"); break;
    default:         DifficultyText = TEXT("Unknown"); break;
    }

    UI_Status(
        FString::Printf(TEXT("[MountainGen] 난이도 변경 → %s"), *DifficultyText),
        2.0f,
        FColor::Green
    );

#if WITH_EDITOR
    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
#endif

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    UWorld* W = GetWorld();
    const bool bEditorLike = (!W || !W->IsGameWorld());

    auto DebugPrintGT = [this](const FString& Msg, float Sec, FColor Col)
        {
            if (IsInGameThread())
            {
                UI_Status(Msg, Sec, Col);
            }
            else
            {
                TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);
                AsyncTask(ENamedThreads::GameThread, [WeakThis, Msg, Sec, Col]()
                    {
                        if (!WeakThis.IsValid()) return;
                        WeakThis->UI_Status(Msg, Sec, Col);
                    });
            }
        };

    auto PrintMetrics = [&](const TCHAR* Prefix, const FMountainGenSettings& SS, const FMGMetrics& MM, float Sec, FColor Col)
        {
            bool okO = false, okS = false;
            const FString Line = FString::Printf(TEXT("%s %s"), Prefix, *MakeMetricsLine(SS, MM, okO, okS));
            DebugPrintGT(Line, Sec, Col);
        };

    // =========================================================
    // 0) Effective Settings
    // =========================================================
    FMountainGenSettings S = Settings;
    MGApplyDifficultyPreset(S);

    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);
    const FVector ActorWorld = GetActorLocation();

    const float FrontX = FMath::Max(200.f, S.CliffThicknessCm);
    const FVector TerrainOriginWorld = ActorWorld - FVector(FrontX, 0.f, 0.f);

    const float Band = FMath::Max(S.CliffDepthCm, Voxel * 2.f);
    const float HalfW = FMath::Max(1.f, S.CliffHalfWidthCm);
    const float H = FMath::Max(1.f, S.CliffHeightCm);

    const float XMinLocal = FrontX - Band;
    const float XMaxLocal = FrontX + Band;
    const float YMinLocal = -HalfW;
    const float YMaxLocal = +HalfW;
    const float ZMinLocal = S.BaseHeightCm;
    const float ZMaxLocal = S.BaseHeightCm + H;

    const FVector SampleOriginWorld = TerrainOriginWorld + FVector(XMinLocal, YMinLocal, ZMinLocal);
    const FVector ChunkOriginWorld = SampleOriginWorld;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = TerrainOriginWorld + FVector(XMaxLocal, YMaxLocal, ZMaxLocal);

    const int32 SampleX = FMath::Max(2, FMath::CeilToInt((XMaxLocal - XMinLocal) / Voxel) + 1);
    const int32 SampleY = FMath::Max(2, FMath::CeilToInt((YMaxLocal - YMinLocal) / Voxel) + 1);
    const int32 SampleZ = FMath::Max(2, FMath::CeilToInt((ZMaxLocal - ZMinLocal) / Voxel) + 1);

    if (S.MetricsStepCm <= 0.f)
        S.MetricsStepCm = FMath::Max(400.f, S.VoxelSizeCm * 2.f);

    const int32 InputSeed = S.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, S.SeedSearchTries);

    if (bDebugPipeline)
    {
        DebugPrintGT(
            FString::Printf(TEXT("[MountainGen] PATH=%s  SeedSearchTries=%d  Sample=%dx%dx%d"),
                bEditorLike ? TEXT("EditorLike") : TEXT("RuntimeAsync"),
                TriesForSeedSearch, SampleX, SampleY, SampleZ),
            4.0f,
            bEditorLike ? FColor::Yellow : FColor::Green
        );
    }

    // =========================================================
    // (A) Editor: 동기 생성
    // =========================================================
    if (bEditorLike)
    {
        if (bDebugPipeline)
        {
            const FMGMetrics M0 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
            PrintMetrics(TEXT("[AutoTune][Before]"), S, M0, 5.0f, FColor::Cyan);
        }

        if (S.bAutoTune)
        {
            MGAutoTuneIntentParams(S, TerrainOriginWorld, WorldMin, WorldMax);
            MGClampToDifficultyBounds(S);
        }

        if (bDebugPipeline)
        {
            const FMGMetrics M1 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
            PrintMetrics(TEXT("[AutoTune][After ]"), S, M1, 5.0f, FColor::Cyan);
        }

        auto DebugPrint = [DebugPrintGT](const FString& Msg, float Sec, FColor Col)
            {
                DebugPrintGT(Msg, Sec, Col);
            };

        const int32 FinalSeed =
            MGSearchSeedForTargets(
                S,
                TerrainOriginWorld,
                WorldMin, WorldMax,
                InputSeed,
                TriesForSeedSearch,
                S.bRetrySeedUntilSatisfied,
                S.MaxSeedAttempts,
                bDebugSeedSearch,
                DebugPrintEveryNAttempt,
                DebugPrint
            );

        S.Seed = FinalSeed;
        MGDeriveReproducibleDomainFromSeed(S, FinalSeed);

        if (bDebugPipeline)
        {
            const FMGMetrics MF = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
            bool okO = false, okS = false;
            const FString Line =
                FString::Printf(TEXT("[FinalMetrics] seed=%d | %s"), S.Seed, *MakeMetricsLine(S, MF, okO, okS));
            DebugPrintGT(Line, 6.0f, (okO && okS) ? FColor::Green : FColor::Orange);
        }

        FVoxelChunk Chunk;
        Chunk.Init(SampleX, SampleY, SampleZ);

        FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

        ParallelFor(SampleZ, [&](int32 z)
            {
                for (int32 y = 0; y < SampleY; ++y)
                    for (int32 x = 0; x < SampleX; ++x)
                    {
                        const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                        Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                    }
            });

        FChunkMeshData MeshData;
        FVoxelMesher::BuildMarchingCubes(
            Chunk,
            S.VoxelSizeCm,
            S.IsoLevel,
            ChunkOriginWorld,
            ActorWorld,
            Gen,
            MeshData
        );

        if (bDebugPipeline)
        {
            DebugPrintGT(
                FString::Printf(TEXT("[Mesh][Raw ] V=%d  T=%d"),
                    MeshData.Vertices.Num(),
                    MeshData.Triangles.Num() / 3),
                4.0f,
                FColor::Silver
            );
        }

        if (bEnablePostWeld)
        {
            MG_WeldVertices_Quantized(MeshData, S.VoxelSizeCm * PostWeldEpsilonScale);
        }

        if (bDebugPipeline && bEnablePostWeld)
        {
            DebugPrintGT(
                FString::Printf(TEXT("[Mesh][Weld] V=%d  T=%d"),
                    MeshData.Vertices.Num(),
                    MeshData.Triangles.Num() / 3),
                4.0f,
                FColor::Silver
            );
        }

        if (bEnableIslandCull)
        {
            MG_CullMeshIslands(MeshData, MinTrisToKeepAfterCull, true);
        }

        // 특정 위치의 이상 음영 원인이 되는 바늘형/퇴화 삼각형을 먼저 제거한다.
        // 기준은 VoxelSize에 비례시켜 형상 손상을 제한한다.
        const int32 RemovedBadTriangles = MG_RemoveBadTriangles(
            MeshData,
            FMath::Square(S.VoxelSizeCm * 0.001f),
            S.VoxelSizeCm * 0.0005f
        );

        // 최종 출력 직전 균열 보정.
        // Marching Cubes를 병렬 slab 단위로 만든 뒤 경계/중복 정점이 남으면 선택 시 노란 선처럼 보이거나
        // Lit 모드에서 미세한 seam/균열이 보일 수 있다.
        // 너무 큰 Weld는 형상을 녹이므로 VoxelSize 기준 소량만 통합한다.
        if (bRepairMeshSeams)
        {
            MG_WeldVertices_Quantized(MeshData, S.VoxelSizeCm * MeshSeamWeldEpsilonScale);
        }

        MG_RecomputeNormalsAndTangents(MeshData);

        if (bDebugPipeline && bEnableIslandCull)
        {
            DebugPrintGT(
                FString::Printf(TEXT("[Mesh][Cull] V=%d  T=%d"),
                    MeshData.Vertices.Num(),
                    MeshData.Triangles.Num() / 3),
                4.0f,
                FColor::Silver
            );
        }

        ProcMesh->ClearAllMeshSections();
        ProcMesh->ClearCollisionConvexMeshes();
        if (TopPlateauMesh)
        {
            TopPlateauMesh->ClearAllMeshSections();
            TopPlateauMesh->ClearCollisionConvexMeshes();
        }

        if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        {
            DebugPrintGT(TEXT("[MountainGen][Editor] MeshData 비어있음 (Cull/Weld로 모두 제거됨)"), 2.0f, FColor::Red);
            return;
        }

        // Build shared surface metadata/report for later placement/gameplay modules.
        GeneratedSurfaceSamples.Reset();
        LastGenerationReport = FMGGenerationReport();
        LastGenerationReport.FinalSeed = S.Seed;

        MG_BuildSurfaceSamples(
            MeshData,
            GetActorLocation(),
            SurfaceSampleTriangleStride,
            MonsterMaxSlopeDeg,
            ItemMaxSlopeDeg,
            PlatformMaxSlopeDeg,
            GeneratedSurfaceSamples
        );

        const FMGMetrics FinalMetricsForReport = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
        MG_FillReportMetrics(S, FinalMetricsForReport, LastGenerationReport);
        MG_AnalyzeMeshQuality(MeshData, RemovedBadTriangles, FMath::Square(S.VoxelSizeCm * 0.08f), LastGenerationReport.MeshQuality);
        MG_BuildZoneReports(GeneratedSurfaceSamples, GenerationZones, GetActorLocation(), LastGenerationReport.ZoneReports);
        LastGenerationReport.SurfaceSampleCount = GeneratedSurfaceSamples.Num();

        ReusableColors.SetNumUninitialized(MeshData.Vertices.Num());

        for (FLinearColor& C : ReusableColors)
        {
            C = FLinearColor::White;
        }

        ProcMesh->CreateMeshSection_LinearColor(
            0,
            MeshData.Vertices,
            MeshData.Triangles,
            MeshData.Normals,
            MeshData.UV0,
            ReusableColors,
            MeshData.Tangents,
            S.bCreateCollision
        );

        ProcMesh->SetCollisionEnabled(
            S.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
        );

        ApplyVoxelMaterialParameters();
        RebuildTopPlateauMesh(MeshData, S);

        Settings.Seed = S.Seed;

        UpdateGeneratedMeshStateAndBroadcast();

        if (bDebugPipeline)
            DebugPrintGT(FString::Printf(TEXT("[MountainGen][Editor] DONE Seed=%d"), Settings.Seed), 2.0f, FColor::Green);

        return;
    }

    // =========================================================
    // (B) Runtime: 비동기
    // =========================================================
    if (bAsyncWorking)
    {
        bRegenQueued = true;
        DebugPrintGT(TEXT("[MountainGen] 작업 중 → 재생성 큐"), 1.5f, FColor::Orange);
        return;
    }

    const bool bLocalEnablePostWeld = bEnablePostWeld;
    const float LocalPostWeldEpsilonScale = PostWeldEpsilonScale;
    const bool bLocalRepairMeshSeams = bRepairMeshSeams;
    const float LocalMeshSeamWeldEpsilonScale = MeshSeamWeldEpsilonScale;
    const bool bLocalEnableIslandCull = bEnableIslandCull;
    const int32 LocalMinTrisToKeepAfterCull = MinTrisToKeepAfterCull;
    const bool bLocalDebugPipeline = bDebugPipeline;
    const bool bLocalDebugSeedSearch = bDebugSeedSearch;
    const int32 LocalDebugPrintEveryNAttempt = DebugPrintEveryNAttempt;

    const int32 LocalBuildSerial = ++CurrentBuildSerial;
    bAsyncWorking = true;
    InFlightBuildSerial = LocalBuildSerial;

    if (bDebugPipeline)
        DebugPrintGT(TEXT("[MountainGen] AutoTune/SeedSearch/생성 시작"), 2.0f, FColor::Cyan);

    TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
        [WeakThis,
        S, TerrainOriginWorld,
        WorldMin, WorldMax,
        ChunkOriginWorld, ActorWorld, SampleOriginWorld,
        SampleX, SampleY, SampleZ, Voxel,
        InputSeed, TriesForSeedSearch, LocalBuildSerial,
        bLocalEnablePostWeld, LocalPostWeldEpsilonScale,
        bLocalRepairMeshSeams, LocalMeshSeamWeldEpsilonScale,
        bLocalEnableIslandCull, LocalMinTrisToKeepAfterCull,
        bLocalDebugPipeline, bLocalDebugSeedSearch, LocalDebugPrintEveryNAttempt]() mutable
        {
            if (!WeakThis.IsValid()) return;

            auto DebugPrint = [WeakThis](const FString& Msg, float Sec, FColor Col)
                {
                    AsyncTask(ENamedThreads::GameThread, [WeakThis, Msg, Sec, Col]()
                        {
                            if (!WeakThis.IsValid()) return;
                            WeakThis->UI_Status(Msg, Sec, Col);
                        });
                };

            // ---- AutoTune ----
            if (bLocalDebugPipeline)
            {
                const FMGMetrics M0 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
                bool okO = false, okS = false;
                DebugPrint(TEXT("[AutoTune][Before] ") + WeakThis->MakeMetricsLine(S, M0, okO, okS), 5.0f, FColor::Cyan);
            }

            if (S.bAutoTune)
            {
                MGAutoTuneIntentParams(S, TerrainOriginWorld, WorldMin, WorldMax);
                MGClampToDifficultyBounds(S);
            }

            if (bLocalDebugPipeline)
            {
                const FMGMetrics M1 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
                bool okO = false, okS = false;
                DebugPrint(TEXT("[AutoTune][After ] ") + WeakThis->MakeMetricsLine(S, M1, okO, okS), 5.0f, FColor::Cyan);
            }

            // ---- Seed Search ----
            const int32 FinalSeed =
                MGSearchSeedForTargets(
                    S,
                    TerrainOriginWorld,
                    WorldMin, WorldMax,
                    InputSeed,
                    TriesForSeedSearch,
                    S.bRetrySeedUntilSatisfied,
                    S.MaxSeedAttempts,
                    bLocalDebugSeedSearch,
                    LocalDebugPrintEveryNAttempt,
                    DebugPrint
                );

            S.Seed = FinalSeed;
            MGDeriveReproducibleDomainFromSeed(S, FinalSeed);

            if (bLocalDebugPipeline)
            {
                const FMGMetrics MF = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
                bool okO = false, okS = false;
                const FString Line =
                    FString::Printf(TEXT("[FinalMetrics] seed=%d | %s"), S.Seed, *WeakThis->MakeMetricsLine(S, MF, okO, okS));
                DebugPrint(Line, 6.0f, (okO && okS) ? FColor::Green : FColor::Orange);
            }

            // ---- Density sampling ----
            FVoxelChunk Chunk;
            Chunk.Init(SampleX, SampleY, SampleZ);

            FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

            // precompute offsets
            TArray<float> XOff; XOff.SetNumUninitialized(SampleX);
            TArray<float> YOff; YOff.SetNumUninitialized(SampleY);
            TArray<float> ZOff; ZOff.SetNumUninitialized(SampleZ);

            for (int32 x = 0; x < SampleX; ++x) XOff[x] = x * Voxel;
            for (int32 y = 0; y < SampleY; ++y) YOff[y] = y * Voxel;
            for (int32 z = 0; z < SampleZ; ++z) ZOff[z] = z * Voxel;

            ParallelFor(SampleZ, [&](int32 z)
                {
                    const float zc = ZOff[z];
                    for (int32 y = 0; y < SampleY; ++y)
                    {
                        const float yc = YOff[y];
                        for (int32 x = 0; x < SampleX; ++x)
                        {
                            const FVector WorldPos = SampleOriginWorld + FVector(XOff[x], yc, zc);
                            Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                        }
                    }
                }, EParallelForFlags::Unbalanced);

            // ---- Meshing / Weld / Cull ----
            FChunkMeshData MeshData;
            FVoxelMesher::BuildMarchingCubes(
                Chunk,
                S.VoxelSizeCm,
                S.IsoLevel,
                ChunkOriginWorld,
                ActorWorld,
                Gen,
                MeshData
            );

            if (bLocalDebugPipeline)
            {
                DebugPrint(
                    FString::Printf(TEXT("[Mesh][Raw ] V=%d  T=%d"),
                        MeshData.Vertices.Num(),
                        MeshData.Triangles.Num() / 3),
                    4.0f,
                    FColor::Silver
                );
            }

            if (bLocalEnablePostWeld)
            {
                MG_WeldVertices_Quantized(MeshData, S.VoxelSizeCm * LocalPostWeldEpsilonScale);

                if (bLocalDebugPipeline)
                {
                    DebugPrint(
                        FString::Printf(TEXT("[Mesh][Weld] V=%d  T=%d"),
                            MeshData.Vertices.Num(),
                            MeshData.Triangles.Num() / 3),
                        4.0f,
                        FColor::Silver
                    );
                }
            }

            if (bLocalEnableIslandCull)
            {
                MG_CullMeshIslands(MeshData, LocalMinTrisToKeepAfterCull, true);

                if (bLocalDebugPipeline)
                {
                    DebugPrint(
                        FString::Printf(TEXT("[Mesh][Cull] V=%d  T=%d"),
                            MeshData.Vertices.Num(),
                            MeshData.Triangles.Num() / 3),
                        4.0f,
                        FColor::Silver
                    );
                }
            }

            // Runtime에서도 Editor와 동일하게 불량 삼각형을 제거한 뒤 노멀/탄젠트를 재계산한다.
            const int32 RemovedBadTriangles = MG_RemoveBadTriangles(
                MeshData,
                FMath::Square(S.VoxelSizeCm * 0.001f),
                S.VoxelSizeCm * 0.0005f
            );

            // Runtime에서도 최종 출력 직전에 같은 Seam Repair를 적용한다.
            if (bLocalRepairMeshSeams)
            {
                MG_WeldVertices_Quantized(MeshData, S.VoxelSizeCm * LocalMeshSeamWeldEpsilonScale);
            }

            MG_RecomputeNormalsAndTangents(MeshData);

            // ---- GameThread apply ----
            AsyncTask(ENamedThreads::GameThread,
                [WeakThis, FinalS = S, MeshData = MoveTemp(MeshData), RemovedBadTriangles, LocalBuildSerial, bLocalDebugPipeline]() mutable
                {
                    if (!WeakThis.IsValid()) return;
                    if (WeakThis->InFlightBuildSerial != LocalBuildSerial) return;

                    FMGAsyncResult Result;
                    Result.bValid = true;
                    Result.BuildSerial = LocalBuildSerial;
                    Result.FinalSettings = FinalS;
                    Result.MeshData = MoveTemp(MeshData);
                    Result.RemovedBadTriangleCount = RemovedBadTriangles;

                    if (bLocalDebugPipeline)
                    {
                        WeakThis->UI_Status(
                            FString::Printf(TEXT("[MountainGen] 시드 확정: %d"), FinalS.Seed),
                            2.5f,
                            FColor::Green
                        );
                    }

                    WeakThis->ApplyGeneratedMeshResult(MoveTemp(Result), true);
                });
        });
}

void AMountainGenWorldActor::UpdateGeneratedMeshStateAndBroadcast()
{
    bHasGeneratedMesh = false;
    GeneratedWorldBounds = FBox(EForceInit::ForceInit);

    if (!ProcMesh)
    {
        return;
    }

    const FBox Box = ProcMesh->Bounds.GetBox();
    if (Box.IsValid == 0)
    {
        return;
    }

    GeneratedWorldBounds = Box;
    bHasGeneratedMesh = true;

    OnMountainGenerated.Broadcast(this);
}