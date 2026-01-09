#include "VoxelMesher.h"
#include "MarchingCubesTables.h"

// ---------- helpers ----------
static FORCEINLINE float SampleSafe(const FVoxelChunk& C, int32 X, int32 Y, int32 Z)
{
    X = FMath::Clamp(X, 0, C.SizeX - 1);
    Y = FMath::Clamp(Y, 0, C.SizeY - 1);
    Z = FMath::Clamp(Z, 0, C.SizeZ - 1);
    return C.Get(X, Y, Z);
}

static FORCEINLINE FVector GradientSafe(const FVoxelChunk& C, int32 X, int32 Y, int32 Z)
{
    const float dx = SampleSafe(C, X + 1, Y, Z) - SampleSafe(C, X - 1, Y, Z);
    const float dy = SampleSafe(C, X, Y + 1, Z) - SampleSafe(C, X, Y - 1, Z);
    const float dz = SampleSafe(C, X, Y, Z + 1) - SampleSafe(C, X, Y, Z - 1);
    return FVector(dx, dy, dz).GetSafeNormal();
}

static FORCEINLINE float SafeT(float Iso, float V1, float V2)
{
    const float Den = (V2 - V1);
    if (FMath::IsNearlyZero(Den)) return 0.0f;
    return (Iso - V1) / Den;
}

// 노멀맵 머티리얼 대응용: 노멀에서 탄젠트 하나 만들어줌
static FORCEINLINE FProcMeshTangent MakeTangentFromNormal(const FVector& N)
{
    // N과 거의 평행하지 않은 축 하나 잡아서 직교기저 만든다
    const FVector Up = (FMath::Abs(N.Z) < 0.99f) ? FVector::UpVector : FVector::RightVector;
    const FVector T = FVector::CrossProduct(Up, N).GetSafeNormal();
    return FProcMeshTangent(T, false);
}

// ---------- main ----------
void FVoxelMesher::BuildMarchingCubes(const FVoxelChunk& Chunk, float VoxelSize, float IsoLevel, FVoxelMeshData& Out)
{
    Out.Reset();

    const int32 MaxX = Chunk.SizeX - 1;
    const int32 MaxY = Chunk.SizeY - 1;
    const int32 MaxZ = Chunk.SizeZ - 1;

    static const int8 EdgeConn[12][2] =
    {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    static const int8 Corner[8][3] =
    {
        {0,0,0},{1,0,0},{1,1,0},{0,1,0},
        {0,0,1},{1,0,1},{1,1,1},{0,1,1}
    };

    FVector VertList[12];
    FVector NormList[12];

    auto AddVertex = [&](const FVector& P, const FVector& N)
        {
            const int32 Idx = Out.Vertices.Num();
            Out.Vertices.Add(P);
            Out.Normals.Add(N);
            Out.UVs.Add(FVector2D(P.X * 0.001f, P.Y * 0.001f));
            Out.Colors.Add(FLinearColor::White);
            Out.Tangents.Add(MakeTangentFromNormal(N));
            return Idx;
        };

    for (int32 z = 0; z < MaxZ; ++z)
        for (int32 y = 0; y < MaxY; ++y)
            for (int32 x = 0; x < MaxX; ++x)
            {
                float Val[8];
                FVector Pos[8];

                for (int32 i = 0; i < 8; ++i)
                {
                    const int32 cx = x + Corner[i][0];
                    const int32 cy = y + Corner[i][1];
                    const int32 cz = z + Corner[i][2];

                    Val[i] = SampleSafe(Chunk, cx, cy, cz);
                    Pos[i] = FVector(cx, cy, cz) * VoxelSize;
                }

                int32 CubeIndex = 0;
                for (int32 i = 0; i < 8; ++i)
                    if (Val[i] < IsoLevel)
                        CubeIndex |= (1 << i);

                const int32 EdgeMask = EdgeTable[CubeIndex];
                if (EdgeMask == 0) continue;

                for (int32 e = 0; e < 12; ++e)
                {
                    if (!(EdgeMask & (1 << e))) continue;

                    const int32 a = EdgeConn[e][0];
                    const int32 b = EdgeConn[e][1];

                    const float t = SafeT(IsoLevel, Val[a], Val[b]);
                    VertList[e] = FMath::Lerp(Pos[a], Pos[b], t);

                    const FVector Na = GradientSafe(Chunk, x + Corner[a][0], y + Corner[a][1], z + Corner[a][2]);
                    const FVector Nb = GradientSafe(Chunk, x + Corner[b][0], y + Corner[b][1], z + Corner[b][2]);

                    NormList[e] = FMath::Lerp(Na, Nb, t).GetSafeNormal();
                }

                for (int32 t = 0; TriTable[CubeIndex][t] != -1; t += 3)
                {
                    const int32 e0 = TriTable[CubeIndex][t + 0];
                    const int32 e1 = TriTable[CubeIndex][t + 1];
                    const int32 e2 = TriTable[CubeIndex][t + 2];

                    const FVector P0 = VertList[e0];
                    const FVector P1 = VertList[e1];
                    const FVector P2 = VertList[e2];

                    FVector N0 = NormList[e0];
                    FVector N1 = NormList[e1];
                    FVector N2 = NormList[e2];

                    // 삼각형 면 노멀과 정점 노멀이 반대면, winding을 뒤집어 자동 교정
                    const FVector FaceN = FVector::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();
                    const FVector AvgN = (N0 + N1 + N2).GetSafeNormal();

                    bool bFlip = (FVector::DotProduct(FaceN, AvgN) < 0.0f);
                    if (bFlip)
                    {
                        // swap 1<->2
                        Swap(P1, P2);
                        Swap(N1, N2);
                    }

                    const int32 i0 = AddVertex(P0, N0);
                    const int32 i1 = AddVertex(P1, N1);
                    const int32 i2 = AddVertex(P2, N2);

                    Out.Triangles.Add(i0);
                    Out.Triangles.Add(i1);
                    Out.Triangles.Add(i2);
                }
            }
}