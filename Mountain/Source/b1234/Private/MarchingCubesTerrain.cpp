#include "MarchingCubesTables.h"
#include "MarchingCubesTerrain.h"
#include "ProceduralMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include <cfloat>


AMarchingCubesTerrain::AMarchingCubesTerrain()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    // 충돌 필요 없으면 false로 시작 (나중에 Marching Cubes 단계에서 켤 수 있음)
    ProcMesh->bUseComplexAsSimpleCollision = false;
}

void AMarchingCubesTerrain::BeginPlay()
{
    Super::BeginPlay();

    // CreateTestTriangle(); // 테스트용

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

    // 최소 구성으로 섹션 생성 (노말/UV는 다음 Step에서)
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;
    Normals.SetNumZeroed(Verts.Num());
    for (int32 i = 0; i < Normals.Num(); ++i)
        Normals[i] = FVector::UpVector;
    UV0.SetNumZeroed(Verts.Num());
    Colors.SetNumZeroed(Verts.Num());
    Tangents.SetNumZeroed(Verts.Num());

    ProcMesh->CreateMeshSection(0, Verts, Tris, Normals, UV0, Colors, Tangents, false);

    UE_LOG(LogTemp, Warning, TEXT("[MC] Built mesh: V=%d  T=%d"), Verts.Num(), Tris.Num() / 3);
}

void AMarchingCubesTerrain::CreateTestTriangle()
{
    // 정점 3개 (단위: cm)
    TArray<FVector> Vertices;
    Vertices.Add(FVector(0, 0, 0));
    Vertices.Add(FVector(0, 300, 0));
    Vertices.Add(FVector(0, 0, 300));

    // 삼각형 인덱스 (0-1-2)
    TArray<int32> Triangles;
    Triangles.Add(0);
    Triangles.Add(1);
    Triangles.Add(2);

    // 노말(라이팅이 보이게)
    TArray<FVector> Normals;
    Normals.Add(FVector(1, 0, 0));
    Normals.Add(FVector(1, 0, 0));
    Normals.Add(FVector(1, 0, 0));

    // UV (머티리얼 테스트용)
    TArray<FVector2D> UV0;
    UV0.Add(FVector2D(0, 0));
    UV0.Add(FVector2D(1, 0));
    UV0.Add(FVector2D(0, 1));

    // 버텍스 컬러 (선택)
    TArray<FColor> VertexColors;
    VertexColors.Add(FColor::Red);
    VertexColors.Add(FColor::Green);
    VertexColors.Add(FColor::Blue);

    // 탄젠트(선택)
    TArray<FProcMeshTangent> Tangents;
    Tangents.Add(FProcMeshTangent(0, 1, 0));
    Tangents.Add(FProcMeshTangent(0, 1, 0));
    Tangents.Add(FProcMeshTangent(0, 1, 0));

    // 섹션 생성
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
    // 아주 단순한 평면: Density = z - Height
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

    // 액터 로컬 기준 점들을 월드로 변환해서 찍기 위해 Transform 사용
    const FTransform& T = GetActorTransform();

    int32 DrawCount = 0;

    for (int32 y = 0; y < NumY; ++y)
        for (int32 x = 0; x < NumX; ++x)
        {
            const float D = Density[Index(x, y, Z)];

            // iso(0) 근처만 찍어서 “표면이 어디쯤인지” 보이게 함
            if (FMath::Abs(D - IsoLevel) <= DebugIsoEpsilon)
            {
                const FVector LocalP = GridToWorld(x, y, Z);
                const FVector WorldP = T.TransformPosition(LocalP);

                // 색은 밀도 부호로 구분: 위(+)/아래(-)
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
    // UE에 PerlinNoise3D가 버전에 따라 없거나 제한적이라 2D 조합으로 근사
    const float n1 = FMath::PerlinNoise2D(FVector2D(X * Freq, Y * Freq));
    const float n2 = FMath::PerlinNoise2D(FVector2D(Y * Freq, Z * Freq));
    const float n3 = FMath::PerlinNoise2D(FVector2D(Z * Freq, X * Freq));
    return (n1 + n2 + n3) / 3.0f; // 대략 -1~1
}

float AMarchingCubesTerrain::HeightFunc_Cliff(const FVector& P) const
{
    // "절벽"은 Height가 급격히 바뀌는 곳이 많아야 함.
    // 핵심 트릭: 노이즈를 "절벽처럼" 만들기 위해 abs+pow로 경사를 세게 만듦.

    // 큰 지형(능선/절벽 위치)
    float n = Noise2D(P.X, P.Y, CliffFreq);       // -1~1
    float a = FMath::Abs(n);                      // 0~1
    float ridge = FMath::Pow(a, 0.25f);           // 0.25~ 절벽처럼(낮은 지수 = 날카롭게)

    // ridge가 클수록(능선/절벽) 높이를 확 올림
    // CliffStrength로 영향 조절
    const float worldHeight = NumZ * CellSize;
    const float base = worldHeight * BaseHeightRatio;

    float h = base + ridge * (worldHeight * 0.6f) * CliffStrength;

    // 표면 디테일(바위 느낌)
    float d = Noise2D(P.X, P.Y, DetailFreq);      // -1~1
    h += d * DetailAmp;

    return h;
}

void AMarchingCubesTerrain::BuildDensityField_CliffRock()
{
    // Density = P.Z - Height(x,y) + (선택) 오버행용 3D 노이즈
    const float worldHeight = NumZ * CellSize;

    for (int32 z = 0; z < NumZ; ++z)
        for (int32 y = 0; y < NumY; ++y)
            for (int32 x = 0; x < NumX; ++x)
            {
                const FVector P = GridToWorld(x, y, z);

                const float H = HeightFunc_Cliff(P);

                float D = P.Z - H; // iso=0이 지면

                if (bEnableOverhang)
                {
                    // 오버행은 "표면 근처"에서만 세게 먹이는 게 안정적
                    // (너무 깊은 내부까지 흔들면 구멍 투성이가 됨)
                    const float surfaceBand = worldHeight * 0.25f; // 표면 근처 범위
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

    // HeightFunc_Cliff는 "월드 높이(cm)"를 반환하므로, 로컬 좌표에서도 그대로 사용 가능
    for (int32 y = 0; y < NumY; y += FMath::Max(1, DebugHeightStep))
        for (int32 x = 0; x < NumX; x += FMath::Max(1, DebugHeightStep))
        {
            const FVector P0 = GridToWorld(x, y, 0);

            // 같은 (x,y)에서 표면 높이
            const float H = HeightFunc_Cliff(P0);

            // 찍을 점: (x,y,H)
            const FVector LocalP = FVector(P0.X, P0.Y, H);
            const FVector WorldP = T.TransformPosition(LocalP);

            DrawDebugPoint(GetWorld(), WorldP, DebugPointSize, FColor::Cyan, false, DebugLifeTime);
            ++DrawCount;
        }

    UE_LOG(LogTemp, Warning, TEXT("[MC] Height debug drew %d points (step=%d)"), DrawCount, DebugHeightStep);
}

FVector AMarchingCubesTerrain::VertexInterp(float Iso, const FVector& P1, const FVector& P2, float V1, float V2) const
{
    // Iso와 거의 같으면 바로 리턴
    if (FMath::Abs(Iso - V1) < KINDA_SMALL_NUMBER) return P1;
    if (FMath::Abs(Iso - V2) < KINDA_SMALL_NUMBER) return P2;

    const float Den = (V2 - V1);
    if (FMath::Abs(Den) < KINDA_SMALL_NUMBER) return P1; // divide-by-zero 방지

    const float T = (Iso - V1) / Den;
    return P1 + T * (P2 - P1);
}

bool AMarchingCubesTerrain::TriangulateCell(int32 X, int32 Y, int32 Z,
    TArray<FVector>& OutVerts,
    TArray<int32>& OutTris) const
{
    // 8 코너 로컬 좌표 오프셋(표준 MC 코너 순서)
    static const int32 CornerOffset[8][3] =
    {
        {0,0,0},{1,0,0},{1,1,0},{0,1,0},
        {0,0,1},{1,0,1},{1,1,1},{0,1,1}
    };

    // edge가 연결하는 코너 인덱스(표준 12 edges)
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

        P[i] = GridToWorld(cx, cy, cz);  // 로컬
        V[i] = Sample(cx, cy, cz);
    }

    // cubeIndex(bitmask)
    int32 CubeIndex = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (V[i] < IsoLevel) CubeIndex |= (1 << i);
    }

    const int32 Edges = FMarchingCubesTables::EdgeTable[CubeIndex];
    if (Edges == 0) return false; // 교차 없음

    FVector VertList[12];

    // 교차하는 edge들의 정점 보간
    for (int e = 0; e < 12; ++e)
    {
        if (Edges & (1 << e))
        {
            const int a = EdgeConnection[e][0];
            const int b = EdgeConnection[e][1];
            VertList[e] = VertexInterp(IsoLevel, P[a], P[b], V[a], V[b]);
        }
    }

    // TriTable 기반으로 triangle 생성
    // TriTable[CubeIndex]는 edge index 3개씩 묶어 triangle을 만든다.
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

        // winding: 기본은 (0,1,2)로 시작. 노말이 뒤집히면 여기서 1,2를 바꿔.
        OutTris.Add(BaseIndex + 0);
        OutTris.Add(BaseIndex + 1);
        OutTris.Add(BaseIndex + 2);
    }

    return true;
}
