#include "VoxelMesher.h"
#include "ProceduralMeshComponent.h"
#include "MarchingCubesTables.h"

static FORCEINLINE bool IsSolid(const FVoxelChunk& C, int32 X, int32 Y, int32 Z)
{
    if (!C.IsInside(X, Y, Z)) return false;
    return C.Get(X, Y, Z) > 0.0f;
}

static FVector VertexInterp(float Iso, const FVector& P1, const FVector& P2, float V1, float V2)
{
    if (FMath::IsNearlyEqual(Iso, V1)) return P1;
    if (FMath::IsNearlyEqual(Iso, V2)) return P2;
    if (FMath::IsNearlyEqual(V1, V2))  return P1;

    const float T = (Iso - V1) / (V2 - V1);
    return P1 + T * (P2 - P1);
}

static float Sample(const FVoxelChunk& C, int32 X, int32 Y, int32 Z)
{
    return C.Get(X, Y, Z);
}

static FVector SampleGradient(const FVoxelChunk& C, int32 X, int32 Y, int32 Z)
{
    const int32 X0 = FMath::Clamp(X - 1, 0, C.SizeX - 1);
    const int32 X1 = FMath::Clamp(X + 1, 0, C.SizeX - 1);
    const int32 Y0 = FMath::Clamp(Y - 1, 0, C.SizeY - 1);
    const int32 Y1 = FMath::Clamp(Y + 1, 0, C.SizeY - 1);
    const int32 Z0 = FMath::Clamp(Z - 1, 0, C.SizeZ - 1);
    const int32 Z1 = FMath::Clamp(Z + 1, 0, C.SizeZ - 1);

    const float Dx = Sample(C, X1, Y, Z) - Sample(C, X0, Y, Z);
    const float Dy = Sample(C, X, Y1, Z) - Sample(C, X, Y0, Z);
    const float Dz = Sample(C, X, Y, Z1) - Sample(C, X, Y, Z0);

    return FVector(Dx, Dy, Dz).GetSafeNormal();
}

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

    auto AddVertex = [&](const FVector& P)
    {
        const int32 Idx = Out.Vertices.Num();
        Out.Vertices.Add(P);

        // 간단 planar UV(나중에 Triplanar 머티리얼 추천)
        Out.UVs.Add(FVector2D(P.X * 0.001f, P.Y * 0.001f));
        Out.Colors.Add(FLinearColor::White);

        return Idx;
     };

    FVector VertList[12];

    for (int32 z = 0; z < MaxZ; ++z)
        for (int32 y = 0; y < MaxY; ++y)
            for (int32 x = 0; x < MaxX; ++x)
            {
                float   Val[8];
                FVector Pos[8];

                for (int32 i = 0; i < 8; ++i)
                {
                    const int32 cx = x + Corner[i][0];
                    const int32 cy = y + Corner[i][1];
                    const int32 cz = z + Corner[i][2];

                    Val[i] = Sample(Chunk, cx, cy, cz);
                    Pos[i] = FVector((float)cx, (float)cy, (float)cz) * VoxelSize;
                }

                // 표준 방식: Val < IsoLevel이면 inside
                int32 CubeIndex = 0;
                for (int32 i = 0; i < 8; ++i)
                {
                    if (Val[i] < IsoLevel) CubeIndex |= (1 << i);
                }

                const int32 EdgeMask = EdgeTable[CubeIndex];
                if (EdgeMask == 0) continue;

                for (int32 e = 0; e < 12; ++e)
                {
                    if (!(EdgeMask & (1 << e))) continue;

                    const int32 a = EdgeConn[e][0];
                    const int32 b = EdgeConn[e][1];

                    VertList[e] = VertexInterp(IsoLevel, Pos[a], Pos[b], Val[a], Val[b]);
                }

                // 삼각형 생성
                for (int32 t = 0; TriTable[CubeIndex][t] != -1; t += 3)
                {
                    const int32 e0 = TriTable[CubeIndex][t + 0];
                    const int32 e1 = TriTable[CubeIndex][t + 1];
                    const int32 e2 = TriTable[CubeIndex][t + 2];

                    const int32 i0 = AddVertex(VertList[e0]);
                    const int32 i1 = AddVertex(VertList[e1]);
                    const int32 i2 = AddVertex(VertList[e2]);

                    Out.Triangles.Add(i0);
                    Out.Triangles.Add(i2);
                    Out.Triangles.Add(i1);
                }
            }

    Out.Normals.Empty();
    Out.Tangents.Empty();
}