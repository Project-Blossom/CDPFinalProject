#include "VoxelMesher.h"
#include "ProceduralMeshComponent.h" // FProcMeshTangent
#include "MarchingCubesTables.h"

static FORCEINLINE bool IsSolid(const FVoxelChunk& C, int32 X, int32 Y, int32 Z)
{
    if (!C.IsInside(X, Y, Z)) return false; // 바깥은 공기 취급(=면 노출)
    return C.Get(X, Y, Z) > 0.0f;
}

// 한 면(Quad) 추가: 외부 노멀 방향 기준으로 CCW 삼각형 생성
static void AddQuad(
    const FVector& A, const FVector& B, const FVector& C, const FVector& D, // Quad corners
    const FVector& Normal,
    FVoxelMeshData& M)
{
    const int32 Start = M.Vertices.Num();

    // 정점
    M.Vertices.Add(A);
    M.Vertices.Add(B);
    M.Vertices.Add(C);
    M.Vertices.Add(D);

    // 노멀
    M.Normals.Add(Normal);
    M.Normals.Add(Normal);
    M.Normals.Add(Normal);
    M.Normals.Add(Normal);

    // UV (면 단위 반복)
    M.UVs.Add(FVector2D(0, 0));
    M.UVs.Add(FVector2D(1, 0));
    M.UVs.Add(FVector2D(1, 1));
    M.UVs.Add(FVector2D(0, 1));

    // Color
    const FLinearColor Col = FLinearColor::White;
    M.Colors.Add(Col); M.Colors.Add(Col); M.Colors.Add(Col); M.Colors.Add(Col);

    // Tangent: 노멀과 직교하는 임의 축
    FVector TangentDir = FVector::CrossProduct(FVector::UpVector, Normal);
    if (TangentDir.IsNearlyZero())
    {
        TangentDir = FVector::CrossProduct(FVector::RightVector, Normal);
    }
    TangentDir.Normalize();
    const FProcMeshTangent T(TangentDir, false);
    M.Tangents.Add(T); M.Tangents.Add(T); M.Tangents.Add(T); M.Tangents.Add(T);

    // 삼각형(두 개) - CCW
    M.Triangles.Add(Start + 0);
    M.Triangles.Add(Start + 1);
    M.Triangles.Add(Start + 2);

    M.Triangles.Add(Start + 0);
    M.Triangles.Add(Start + 2);
    M.Triangles.Add(Start + 3);
}

void FVoxelMesher::BuildSurface(const FVoxelChunk& Chunk, float VoxelSize, FVoxelMeshData& Out)
{
    Out.Reset();

    // 각 복셀의 월드 위치
    auto V = [VoxelSize](int32 X, int32 Y, int32 Z)
        {
            return FVector((float)X * VoxelSize, (float)Y * VoxelSize, (float)Z * VoxelSize);
        };

    for (int32 z = 0; z < Chunk.SizeZ; ++z)
    {
        for (int32 y = 0; y < Chunk.SizeY; ++y)
        {
            for (int32 x = 0; x < Chunk.SizeX; ++x)
            {
                if (!IsSolid(Chunk, x, y, z))
                    continue;

                // 현재 복셀 큐브 8코너
                const FVector P000 = V(x, y, z);
                const FVector P100 = V(x + 1, y, z);
                const FVector P010 = V(x, y + 1, z);
                const FVector P110 = V(x + 1, y + 1, z);

                const FVector P001 = V(x, y, z + 1);
                const FVector P101 = V(x + 1, y, z + 1);
                const FVector P011 = V(x, y + 1, z + 1);
                const FVector P111 = V(x + 1, y + 1, z + 1);

                // +X 면: 이웃 (x+1,y,z)가 공기면 노출
                if (!IsSolid(Chunk, x + 1, y, z))
                {
                    // 바깥을 +X로 보는 면의 CCW 순서
                    AddQuad(P100, P101, P111, P110, FVector(1, 0, 0), Out);
                }

                // -X
                if (!IsSolid(Chunk, x - 1, y, z))
                {
                    AddQuad(P000, P010, P011, P001, FVector(-1, 0, 0), Out);
                }

                // +Y
                if (!IsSolid(Chunk, x, y + 1, z))
                {
                    AddQuad(P010, P110, P111, P011, FVector(0, 1, 0), Out);
                }

                // -Y
                if (!IsSolid(Chunk, x, y - 1, z))
                {
                    AddQuad(P000, P001, P101, P100, FVector(0, -1, 0), Out);
                }

                // +Z (위)
                if (!IsSolid(Chunk, x, y, z + 1))
                {
                    AddQuad(P001, P011, P111, P101, FVector(0, 0, 1), Out);
                }

                // -Z (아래)
                if (!IsSolid(Chunk, x, y, z - 1))
                {
                    AddQuad(P000, P100, P110, P010, FVector(0, 0, -1), Out);
                }
            }
        }
    }
}

//--

static FORCEINLINE int32 Idx(int32 X, int32 Y, int32 NumX)
{
    return X + Y * NumX;
}

// (x,y)에서 가장 높은 Solid z를 찾는다. 없으면 -1
static int32 FindTopSolidZ(const FVoxelChunk& Chunk, int32 X, int32 Y)
{
    for (int32 Z = Chunk.SizeZ - 1; Z >= 0; --Z)
    {
        if (Chunk.Get(X, Y, Z) > 0.0f) // Solid 기준
        {
            return Z;
        }
    }
    return -1;
}

void FVoxelMesher::BuildHeightfieldSurface(const FVoxelChunk& Chunk, float VoxelSize, FVoxelMeshData& Out)
{
    Out.Reset();

    const int32 NumX = Chunk.SizeX;
    const int32 NumY = Chunk.SizeY;

    const int32 NumVertsX = NumX + 1;
    const int32 NumVertsY = NumY + 1;
    const int32 NumVerts = NumVertsX * NumVertsY;

    Out.Vertices.SetNum(NumVerts);
    Out.Normals.SetNum(NumVerts);
    Out.UVs.SetNum(NumVerts);
    Out.Colors.SetNum(NumVerts);
    Out.Tangents.SetNum(NumVerts);

    // 1) 정점 배치: (x,y)마다 top z를 찾아서 높이로 사용
    for (int32 Y = 0; Y < NumVertsY; ++Y)
    {
        for (int32 X = 0; X < NumVertsX; ++X)
        {
            // 경계는 이웃 샘플로 처리(간단히 clamp)
            const int32 SX = FMath::Clamp(X, 0, NumX - 1);
            const int32 SY = FMath::Clamp(Y, 0, NumY - 1);

            const int32 TopZ = FindTopSolidZ(Chunk, SX, SY);
            const float Height = (TopZ >= 0 ? (TopZ + 1) * VoxelSize : 0.0f);

            const int32 V = Idx(X, Y, NumVertsX);
            Out.Vertices[V] = FVector((float)X * VoxelSize, (float)Y * VoxelSize, Height);
            Out.UVs[V] = FVector2D((float)X / (float)NumX, (float)Y / (float)NumY);
            Out.Colors[V] = FLinearColor::White;
            Out.Normals[V] = FVector::ZeroVector;
            Out.Tangents[V] = FProcMeshTangent(FVector::RightVector, false);
        }
    }

    // 2) 삼각형 인덱스
    for (int32 Y = 0; Y < NumY; ++Y)
    {
        for (int32 X = 0; X < NumX; ++X)
        {
            const int32 I00 = Idx(X, Y, NumVertsX);
            const int32 I10 = Idx(X + 1, Y, NumVertsX);
            const int32 I01 = Idx(X, Y + 1, NumVertsX);
            const int32 I11 = Idx(X + 1, Y + 1, NumVertsX);

            // CCW
            Out.Triangles.Add(I00); Out.Triangles.Add(I11); Out.Triangles.Add(I10);
            Out.Triangles.Add(I00); Out.Triangles.Add(I01); Out.Triangles.Add(I11);
        }
    }

    // 3) 노멀 계산(부드러운 산)
    for (int32 i = 0; i < Out.Triangles.Num(); i += 3)
    {
        const int32 IA = Out.Triangles[i];
        const int32 IB = Out.Triangles[i + 1];
        const int32 IC = Out.Triangles[i + 2];

        const FVector& A = Out.Vertices[IA];
        const FVector& B = Out.Vertices[IB];
        const FVector& C = Out.Vertices[IC];

        const FVector N = FVector::CrossProduct(C - A, B - A).GetSafeNormal();

        Out.Normals[IA] += N;
        Out.Normals[IB] += N;
        Out.Normals[IC] += N;
    }

    for (FVector& N : Out.Normals)
    {
        N.Normalize();
    }
}

//--

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
    // 중앙차분(경계는 clamp)로 노멀용 그라디언트
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

    // AddVertex: Normals는 여기서 넣어도 되지만,
    // 우리는 마지막에 "한 번" 재계산할 거라 0으로 넣어도 됨.
    auto AddVertex = [&](const FVector& P)
        {
            const int32 Idx = Out.Vertices.Num();
            Out.Vertices.Add(P);
            Out.Normals.Add(FVector::ZeroVector); // ✅ 최종에 재계산
            Out.UVs.Add(FVector2D(P.X * 0.001f, P.Y * 0.001f));
            Out.Colors.Add(FLinearColor::White);
            Out.Tangents.Add(FProcMeshTangent(FVector::RightVector, false));
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

                // ✅ 2번 방식으로 통일: Val > IsoLevel 을 inside로
                int32 CubeIndex = 0;
                for (int32 i = 0; i < 8; ++i)
                {
                    if (Val[i] > IsoLevel) CubeIndex |= (1 << i);
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

                for (int32 t = 0; TriTable[CubeIndex][t] != -1; t += 3)
                {
                    const int32 e0 = TriTable[CubeIndex][t + 0];
                    const int32 e1 = TriTable[CubeIndex][t + 1];
                    const int32 e2 = TriTable[CubeIndex][t + 2];

                    const int32 i0 = AddVertex(VertList[e0]);
                    const int32 i1 = AddVertex(VertList[e1]);
                    const int32 i2 = AddVertex(VertList[e2]);

                    // ✅ CubeIndex를 (Val > Iso)로 통일했으니, 삼각형은 표준 CCW로
                    Out.Triangles.Add(i0);
                    Out.Triangles.Add(i1);
                    Out.Triangles.Add(i2);
                }
            }

    // ✅ 노멀 재계산은 "전체 메쉬 완성 후" 딱 1번만
    Out.Normals.SetNumZeroed(Out.Vertices.Num());

    for (int32 i = 0; i < Out.Triangles.Num(); i += 3)
    {
        const int32 IA = Out.Triangles[i + 0];
        const int32 IB = Out.Triangles[i + 1];
        const int32 IC = Out.Triangles[i + 2];

        const FVector& A = Out.Vertices[IA];
        const FVector& B = Out.Vertices[IB];
        const FVector& C = Out.Vertices[IC];

        FVector FaceN = FVector::CrossProduct(B - A, C - A);
        const float LenSq = FaceN.SizeSquared();
        if (LenSq < KINDA_SMALL_NUMBER) continue; // ✅ 퇴화 삼각형 방지

        FaceN *= FMath::InvSqrt(LenSq);

        Out.Normals[IA] += FaceN;
        Out.Normals[IB] += FaceN;
        Out.Normals[IC] += FaceN;
    }

    for (FVector& N : Out.Normals)
    {
        if (!N.Normalize())
        {
            N = FVector::UpVector; // ✅ 0노멀 방지
        }
    }
}