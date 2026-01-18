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
    BuildDensityField_TestPlane();
    LogDensityRange();

    if (bDebugDrawSlice)
    {
        DebugDrawDensitySlice();
    }

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
