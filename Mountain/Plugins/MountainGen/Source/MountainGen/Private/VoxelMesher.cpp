#include "VoxelMesher.h"
#include "VoxelChunk.h"
#include "MountainGenMeshData.h"
#include "MarchingCubesTables.h"

static FORCEINLINE FVector LerpVertex(const FVector& P1, const FVector& P2, float V1, float V2, float Iso)
{
    const float Den = (V2 - V1);
    if (FMath::IsNearlyZero(Den)) return P1;
    const float T = (Iso - V1) / Den;
    return P1 + T * (P2 - P1);
}

static FORCEINLINE int32 Idx3(int32 X, int32 Y, int32 Z, int32 SX, int32 SY)
{
    return X + Y * SX + Z * SX * SY;
}

void FVoxelMesher::BuildMarchingCubes(
    const FVoxelChunk& Chunk,
    float VoxelSizeCm,
    float IsoLevel,
    const FVector& ChunkOriginWorld,
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

    // 셀 개수는 (샘플-1)
    const int32 CX = SX - 1;
    const int32 CY = SY - 1;
    const int32 CZ = SZ - 1;

    // 에지 정점 보간
    FVector VertList[12];

    auto Sample = [&](int32 x, int32 y, int32 z) -> float
        {
            return Chunk.Density[Idx3(x, y, z, SX, SY)];
        };

    auto WorldP = [&](int32 x, int32 y, int32 z) -> FVector
        {
            return ChunkOriginWorld + FVector(x * VoxelSizeCm, y * VoxelSizeCm, z * VoxelSizeCm);
        };

    for (int32 z = 0; z < CZ; ++z)
        for (int32 y = 0; y < CY; ++y)
            for (int32 x = 0; x < CX; ++x)
            {
                // 코너 8개
                const float V0 = Sample(x, y, z);
                const float V1 = Sample(x + 1, y, z);
                const float V2 = Sample(x + 1, y + 1, z);
                const float V3 = Sample(x, y + 1, z);
                const float V4 = Sample(x, y, z + 1);
                const float V5 = Sample(x + 1, y, z + 1);
                const float V6 = Sample(x + 1, y + 1, z + 1);
                const float V7 = Sample(x, y + 1, z + 1);

                int32 CubeIndex = 0;
                if (V0 < IsoLevel) CubeIndex |= 1;
                if (V1 < IsoLevel) CubeIndex |= 2;
                if (V2 < IsoLevel) CubeIndex |= 4;
                if (V3 < IsoLevel) CubeIndex |= 8;
                if (V4 < IsoLevel) CubeIndex |= 16;
                if (V5 < IsoLevel) CubeIndex |= 32;
                if (V6 < IsoLevel) CubeIndex |= 64;
                if (V7 < IsoLevel) CubeIndex |= 128;

                const int32 Edges = EdgeTable[CubeIndex];
                if (Edges == 0) continue;

                const FVector P0 = WorldP(x, y, z);
                const FVector P1w = WorldP(x + 1, y, z);
                const FVector P2w = WorldP(x + 1, y + 1, z);
                const FVector P3w = WorldP(x, y + 1, z);
                const FVector P4w = WorldP(x, y, z + 1);
                const FVector P5w = WorldP(x + 1, y, z + 1);
                const FVector P6w = WorldP(x + 1, y + 1, z + 1);
                const FVector P7w = WorldP(x, y + 1, z + 1);

                if (Edges & 1)   VertList[0] = LerpVertex(P0, P1w, V0, V1, IsoLevel);
                if (Edges & 2)   VertList[1] = LerpVertex(P1w, P2w, V1, V2, IsoLevel);
                if (Edges & 4)   VertList[2] = LerpVertex(P2w, P3w, V2, V3, IsoLevel);
                if (Edges & 8)   VertList[3] = LerpVertex(P3w, P0, V3, V0, IsoLevel);
                if (Edges & 16)  VertList[4] = LerpVertex(P4w, P5w, V4, V5, IsoLevel);
                if (Edges & 32)  VertList[5] = LerpVertex(P5w, P6w, V5, V6, IsoLevel);
                if (Edges & 64)  VertList[6] = LerpVertex(P6w, P7w, V6, V7, IsoLevel);
                if (Edges & 128) VertList[7] = LerpVertex(P7w, P4w, V7, V4, IsoLevel);
                if (Edges & 256) VertList[8] = LerpVertex(P0, P4w, V0, V4, IsoLevel);
                if (Edges & 512) VertList[9] = LerpVertex(P1w, P5w, V1, V5, IsoLevel);
                if (Edges & 1024)VertList[10] = LerpVertex(P2w, P6w, V2, V6, IsoLevel);
                if (Edges & 2048)VertList[11] = LerpVertex(P3w, P7w, V3, V7, IsoLevel);

                // 삼각형 생성
                for (int32 i = 0; TriTable[CubeIndex][i] != -1; i += 3)
                {
                    const int32 A = TriTable[CubeIndex][i];
                    const int32 B = TriTable[CubeIndex][i + 1];
                    const int32 Cc = TriTable[CubeIndex][i + 2];

                    const int32 BaseIndex = Out.Vertices.Num();
                    const FVector VA = VertList[A];
                    const FVector VB = VertList[B];
                    const FVector VC = VertList[Cc];

                    Out.Vertices.Add(VA);
                    Out.Vertices.Add(VB);
                    Out.Vertices.Add(VC);

                    // winding
                    Out.Triangles.Add(BaseIndex + 0);
                    Out.Triangles.Add(BaseIndex + 1);
                    Out.Triangles.Add(BaseIndex + 2);

                    // 노말
                    const FVector N = FVector::CrossProduct(VB - VA, VC - VA).GetSafeNormal();
                    Out.Normals.Add(N);
                    Out.Normals.Add(N);
                    Out.Normals.Add(N);

                    // UV
                    Out.UV0.Add(FVector2D(0, 0));
                    Out.UV0.Add(FVector2D(1, 0));
                    Out.UV0.Add(FVector2D(0, 1));

                    // Tangent
                    Out.Tangents.Add(FProcMeshTangent(1, 0, 0));
                    Out.Tangents.Add(FProcMeshTangent(1, 0, 0));
                    Out.Tangents.Add(FProcMeshTangent(1, 0, 0));
                }
            }
}