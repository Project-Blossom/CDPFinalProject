#include "MarchingCubesTerrain.h"
#include "MarchingCubesTables.h"
#include "ProceduralMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include <cfloat>


AMarchingCubesTerrain::AMarchingCubesTerrain()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    // 그림자 캐스팅 활성화 — 빛 투과 방지
    ProcMesh->SetCastShadow(true);
    ProcMesh->bCastDynamicShadow = true;

    // �浹 �ʿ� ������ false�� ���� (���߿� Marching Cubes �ܰ迡�� �� �� ����)
    ProcMesh->bUseComplexAsSimpleCollision = false;
}

void AMarchingCubesTerrain::BeginPlay()
{
    Super::BeginPlay();

    // CreateTestTriangle(); // �׽�Ʈ��

    AllocateDensity();
    BuildDensityField_CliffRock();
    LogDensityRange();

    if (bDebugDrawSlice)
    {
        DebugDrawDensitySlice();
    }
    if (bDebugDrawHeight)
    {
        DebugDrawHeightPoints();
    }

    TArray<FVector> Verts;
    TArray<int32> Tris;

    for (int32 z = 0; z < NumZ - 1; ++z)
        for (int32 y = 0; y < NumY - 1; ++y)
            for (int32 x = 0; x < NumX - 1; ++x)
            {
                TriangulateCell(x, y, z, Verts, Tris);
            }

    // �ּ� �������� ���� ���� (�븻/UV�� ���� Step����)
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    // 트라이앵글 기반 버텍스 노멀 계산
    // 각 삼각형의 면 노멀을 계산하여 공유 버텍스에 누적 후 정규화
    Normals.SetNumZeroed(Verts.Num());
    UV0.SetNumZeroed(Verts.Num());
    Colors.SetNumZeroed(Verts.Num());
    Tangents.SetNumZeroed(Verts.Num());

    // 삼각형마다 면 노멀(Cross Product) 계산 후 버텍스별 누적
    for (int32 i = 0; i < Tris.Num(); i += 3)
    {
        const int32 I0 = Tris[i];
        const int32 I1 = Tris[i + 1];
        const int32 I2 = Tris[i + 2];

        const FVector& V0 = Verts[I0];
        const FVector& V1 = Verts[I1];
        const FVector& V2 = Verts[I2];

        // 면 노멀 = (V2-V0) × (V1-V0) — 와인딩 (0,2,1) 기준 외적
        const FVector FaceNormal = FVector::CrossProduct(V2 - V0, V1 - V0);

        Normals[I0] += FaceNormal;
        Normals[I1] += FaceNormal;
        Normals[I2] += FaceNormal;
    }

    // 누적 노멀 정규화 (실패 시 UpVector fallback)
    for (FVector& N : Normals)
    {
        N = N.GetSafeNormal();
        if (N.IsNearlyZero())
        {
            N = FVector::UpVector;
        }
    }

    // 누적 노멀 정규화 (실패 시 UpVector fallback)
    for (FVector& N : Normals)
    {
        N = N.GetSafeNormal();
        if (N.IsNearlyZero())
        {
            N = FVector::UpVector;
        }
    }

    ProcMesh->CreateMeshSection(0, Verts, Tris, Normals, UV0, Colors, Tangents, false);

    UE_LOG(LogTemp, Warning, TEXT("[MC] Built mesh: V=%d  T=%d"), Verts.Num(), Tris.Num() / 3);
}

void AMarchingCubesTerrain::CreateTestTriangle()
{
    // ���� 3�� (����: cm)
    TArray<FVector> Vertices;
    Vertices.Add(FVector(0, 0, 0));
    Vertices.Add(FVector(0, 300, 0));
    Vertices.Add(FVector(0, 0, 300));

    // �ﰢ�� �ε��� (0-1-2)
    TArray<int32> Triangles;
    Triangles.Add(0);
    Triangles.Add(1);
    Triangles.Add(2);

    // �븻(�������� ���̰�)
    TArray<FVector> Normals;
    Normals.Add(FVector(1, 0, 0));
    Normals.Add(FVector(1, 0, 0));
    Normals.Add(FVector(1, 0, 0));

    // UV (��Ƽ���� �׽�Ʈ��)
    TArray<FVector2D> UV0;
    UV0.Add(FVector2D(0, 0));
    UV0.Add(FVector2D(1, 0));
    UV0.Add(FVector2D(0, 1));

    // ���ؽ� �÷� (����)
    TArray<FColor> VertexColors;
    VertexColors.Add(FColor::Red);
    VertexColors.Add(FColor::Green);
    VertexColors.Add(FColor::Blue);

    // ź��Ʈ(����)
    TArray<FProcMeshTangent> Tangents;
    Tangents.Add(FProcMeshTangent(0, 1, 0));
    Tangents.Add(FProcMeshTangent(0, 1, 0));
    Tangents.Add(FProcMeshTangent(0, 1, 0));

    // ���� ����
    const int32 SectionIndex = 0;
    const bool bCreateCollision = false;

    ProcMesh->CreateMeshSection(
        SectionIndex,
        Vertices,
        Triangles,
        Normals,
        UV0,
        VertexColors,
        Tangents,
        bCreateCollision
    );

}
void AMarchingCubesTerrain::AllocateDensity()
{
    NumX = FMath::Max(2, NumX);
    NumY = FMath::Max(2, NumY);
    NumZ = FMath::Max(2, NumZ);

    const int32 Total = NumX * NumY * NumZ;
    Density.SetNumZeroed(Total);
}

void AMarchingCubesTerrain::BuildDensityField_TestPlane()
{
    // ���� �ܼ��� ���: Density = z - Height
    const float Height = (NumZ * CellSize) * 0.4f;

    for (int32 z = 0; z < NumZ; ++z)
        for (int32 y = 0; y < NumY; ++y)
            for (int32 x = 0; x < NumX; ++x)
            {
                const FVector P = GridToWorld(x, y, z);
                Density[Index(x, y, z)] = P.Z - Height;
            }
}

void AMarchingCubesTerrain::LogDensityRange() const
{
    float MinV = FLT_MAX;
    float MaxV = -FLT_MAX;

    for (float V : Density)
    {
        MinV = FMath::Min(MinV, V);
        MaxV = FMath::Max(MaxV, V);
    }

    UE_LOG(LogTemp, Warning, TEXT("[MC] Density range: %f ~ %f (Iso=%f)"), MinV, MaxV, IsoLevel);
}

void AMarchingCubesTerrain::DebugDrawDensitySlice() const
{
    if (!GetWorld()) return;
    if (Density.Num() == 0) return;

    const int32 Z = FMath::Clamp(DebugSliceZ, 0, NumZ - 1);

    // ���� ���� ���� ������ ����� ��ȯ�ؼ� ��� ���� Transform ���
    const FTransform& T = GetActorTransform();

    int32 DrawCount = 0;

    for (int32 y = 0; y < NumY; ++y)
        for (int32 x = 0; x < NumX; ++x)
        {
            const float D = Density[Index(x, y, Z)];

            // iso(0) ��ó�� �� ��ǥ���� ����������� ���̰� ��
            if (FMath::Abs(D - IsoLevel) <= DebugIsoEpsilon)
            {
                const FVector LocalP = GridToWorld(x, y, Z);
                const FVector WorldP = T.TransformPosition(LocalP);

                // ���� �е� ��ȣ�� ����: ��(+)/�Ʒ�(-)
                const FColor C = (D >= IsoLevel) ? FColor::Green : FColor::Red;

                DrawDebugPoint(GetWorld(), WorldP, DebugPointSize, C, false, DebugLifeTime);
                ++DrawCount;
            }
        }

    UE_LOG(LogTemp, Warning, TEXT("[MC] Debug slice Z=%d drew %d points (eps=%f)"),
        Z, DrawCount, DebugIsoEpsilon);
}

float AMarchingCubesTerrain::Noise2D(float X, float Y, float Freq) const
{
    return FMath::PerlinNoise2D(FVector2D(X * Freq, Y * Freq)); // -1~1
}

float AMarchingCubesTerrain::Noise3D(float X, float Y, float Z, float Freq) const
{
    // UE�� PerlinNoise3D�� ������ ���� ���ų� �������̶� 2D �������� �ٻ�
    const float n1 = FMath::PerlinNoise2D(FVector2D(X * Freq, Y * Freq));
    const float n2 = FMath::PerlinNoise2D(FVector2D(Y * Freq, Z * Freq));
    const float n3 = FMath::PerlinNoise2D(FVector2D(Z * Freq, X * Freq));
    return (n1 + n2 + n3) / 3.0f; // �뷫 -1~1
}

float AMarchingCubesTerrain::HeightFunc_Cliff(const FVector& P) const
{
    // "����"�� Height�� �ް��� �ٲ�� ���� ���ƾ� ��.
    // �ٽ� Ʈ��: ����� "����ó��" ����� ���� abs+pow�� ��縦 ���� ����.

    // ū ����(�ɼ�/���� ��ġ)
    float n = Noise2D(P.X, P.Y, CliffFreq);       // -1~1
    float a = FMath::Abs(n);                      // 0~1
    float ridge = FMath::Pow(a, 0.25f);           // 0.25~ ����ó��(���� ���� = ��ī�Ӱ�)

    // ridge�� Ŭ����(�ɼ�/����) ���̸� Ȯ �ø�
    // CliffStrength�� ���� ����
    const float worldHeight = NumZ * CellSize;
    const float base = worldHeight * BaseHeightRatio;

    float h = base + ridge * (worldHeight * 0.6f) * CliffStrength;

    // ǥ�� ������(���� ����)
    float d = Noise2D(P.X, P.Y, DetailFreq);      // -1~1
    h += d * DetailAmp;

    return h;
}

void AMarchingCubesTerrain::BuildDensityField_CliffRock()
{
    // Density = P.Z - Height(x,y) + (����) ������� 3D ������
    const float worldHeight = NumZ * CellSize;

    for (int32 z = 0; z < NumZ; ++z)
        for (int32 y = 0; y < NumY; ++y)
            for (int32 x = 0; x < NumX; ++x)
            {
                const FVector P = GridToWorld(x, y, z);

                const float H = HeightFunc_Cliff(P);

                float D = P.Z - H; // iso=0�� ����

                if (bEnableOverhang)
                {
                    // �������� "ǥ�� ��ó"������ ���� ���̴� �� ������
                    // (�ʹ� ���� ���α��� ���� ���� �����̰� ��)
                    const float surfaceBand = worldHeight * 0.25f; // ǥ�� ��ó ����
                    const float band = 1.0f - FMath::Clamp(FMath::Abs(D) / surfaceBand, 0.0f, 1.0f);

                    const float o = Noise3D(P.X, P.Y, P.Z, OverhangFreq); // -1~1
                    D += o * OverhangAmp * band;
                }

                Density[Index(x, y, z)] = D;
            }
}

void AMarchingCubesTerrain::DebugDrawHeightPoints() const
{
    if (!GetWorld()) return;
    const FTransform& T = GetActorTransform();

    int32 DrawCount = 0;

    // HeightFunc_Cliff�� "���� ����(cm)"�� ��ȯ�ϹǷ�, ���� ��ǥ������ �״�� ��� ����
    for (int32 y = 0; y < NumY; y += FMath::Max(1, DebugHeightStep))
        for (int32 x = 0; x < NumX; x += FMath::Max(1, DebugHeightStep))
        {
            const FVector P0 = GridToWorld(x, y, 0);

            // ���� (x,y)���� ǥ�� ����
            const float H = HeightFunc_Cliff(P0);

            // ���� ��: (x,y,H)
            const FVector LocalP = FVector(P0.X, P0.Y, H);
            const FVector WorldP = T.TransformPosition(LocalP);

            DrawDebugPoint(GetWorld(), WorldP, DebugPointSize, FColor::Cyan, false, DebugLifeTime);
            ++DrawCount;
        }

    UE_LOG(LogTemp, Warning, TEXT("[MC] Height debug drew %d points (step=%d)"), DrawCount, DebugHeightStep);
}

FVector AMarchingCubesTerrain::VertexInterp(float Iso, const FVector& P1, const FVector& P2, float V1, float V2) const
{
    // Iso�� ���� ������ �ٷ� ����
    if (FMath::Abs(Iso - V1) < KINDA_SMALL_NUMBER) return P1;
    if (FMath::Abs(Iso - V2) < KINDA_SMALL_NUMBER) return P2;

    const float Den = (V2 - V1);
    if (FMath::Abs(Den) < KINDA_SMALL_NUMBER) return P1; // divide-by-zero ����

    const float T = (Iso - V1) / Den;
    return P1 + T * (P2 - P1);
}

bool AMarchingCubesTerrain::TriangulateCell(int32 X, int32 Y, int32 Z,
    TArray<FVector>& OutVerts,
    TArray<int32>& OutTris) const
{
    // 8 �ڳ� ���� ��ǥ ������(ǥ�� MC �ڳ� ����)
    static const int32 CornerOffset[8][3] =
    {
        {0,0,0},{1,0,0},{1,1,0},{0,1,0},
        {0,0,1},{1,0,1},{1,1,1},{0,1,1}
    };

    // edge�� �����ϴ� �ڳ� �ε���(ǥ�� 12 edges)
    static const int32 EdgeConnection[12][2] =
    {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    FVector P[8];
    float   V[8];

    for (int i = 0; i < 8; ++i)
    {
        const int32 cx = X + CornerOffset[i][0];
        const int32 cy = Y + CornerOffset[i][1];
        const int32 cz = Z + CornerOffset[i][2];

        P[i] = GridToWorld(cx, cy, cz);  // ����
        V[i] = Sample(cx, cy, cz);
    }

    // cubeIndex(bitmask)
    int32 CubeIndex = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (V[i] < IsoLevel) CubeIndex |= (1 << i);
    }

    const int32 Edges = FMarchingCubesTables::EdgeTable[CubeIndex];
    if (Edges == 0) return false; // ���� ����

    FVector VertList[12];

    // �����ϴ� edge���� ���� ����
    for (int e = 0; e < 12; ++e)
    {
        if (Edges & (1 << e))
        {
            const int a = EdgeConnection[e][0];
            const int b = EdgeConnection[e][1];
            VertList[e] = VertexInterp(IsoLevel, P[a], P[b], V[a], V[b]);
        }
    }

    // TriTable ������� triangle ����
    // TriTable[CubeIndex]�� edge index 3���� ���� triangle�� �����.
    for (int t = 0; t < 16; t += 3)
    {
        const int32 A = FMarchingCubesTables::TriTable[CubeIndex][t];
        if (A == -1) break;
        const int32 B = FMarchingCubesTables::TriTable[CubeIndex][t + 1];
        const int32 C = FMarchingCubesTables::TriTable[CubeIndex][t + 2];

        const int32 BaseIndex = OutVerts.Num();

        OutVerts.Add(VertList[A]);
        OutVerts.Add(VertList[B]);
        OutVerts.Add(VertList[C]);

        // winding: (0,2,1) — 노멀이 암벽 바깥쪽을 향하도록 반전
        OutTris.Add(BaseIndex + 0);
        OutTris.Add(BaseIndex + 2);
        OutTris.Add(BaseIndex + 1);
    }

    return true;
}
