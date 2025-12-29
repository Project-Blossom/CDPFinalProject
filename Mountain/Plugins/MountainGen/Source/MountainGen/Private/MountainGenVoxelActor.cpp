#include "MountainGenVoxelActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"

AMountainGenVoxelActor::AMountainGenVoxelActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = false; // 디버그/안정 우선
}

void AMountainGenVoxelActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    BuildChunkAndMesh();
}

void AMountainGenVoxelActor::Regenerate()
{
    BuildChunkAndMesh();
}

void AMountainGenVoxelActor::SetSeed(int32 NewSeed)
{
    Seed = NewSeed;
    BuildChunkAndMesh();
}

void AMountainGenVoxelActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    ProcMesh->ClearAllMeshSections();

    // 1) Chunk 생성
    FVoxelChunk Chunk(ChunkX, ChunkY, ChunkZ);

    // 2) Density 채우기 (Seed 기반)
    FVoxelDensityGenerator Gen(Seed);
    Gen.HeightScale = HeightScale;
    Gen.HeightAmp = HeightAmp;
    Gen.CaveScale = CaveScale;
    Gen.CaveStrength = CaveStrength;
    Gen.BaseFloor = BaseFloor;

    for (int32 z = 0; z < ChunkZ; ++z)
    {
        for (int32 y = 0; y < ChunkY; ++y)
        {
            for (int32 x = 0; x < ChunkX; ++x)
            {
                const float D = Gen.GetDensity(x, y, z);
                Chunk.Set(x, y, z, D);
            }
        }
    }

    // 3) 노출면만 메쉬 생성
    FVoxelMeshData Mesh;
    // FVoxelMesher::BuildSurface(Chunk, VoxelSize, Mesh);
    //FVoxelMesher::BuildHeightfieldSurface(Chunk, VoxelSize, Mesh);
    FVoxelMesher::BuildMarchingCubes(Chunk, VoxelSize, 0.0f, Mesh);

    // 4) ProceduralMesh에 업로드 + 콜리전 생성
    ProcMesh->CreateMeshSection_LinearColor(
        0,
        Mesh.Vertices,
        Mesh.Triangles,
        Mesh.Normals,
        Mesh.UVs,
        Mesh.Colors,
        Mesh.Tangents,
        true // bCreateCollision
    );

    ProcMesh->bUseComplexAsSimpleCollision = true;

    if (VoxelMaterial)
    {
        ProcMesh->SetMaterial(0, VoxelMaterial);
    }
}

//#include "MountainGenVoxelActor.h"
//
//#include "ProceduralMeshComponent.h"
//#include "Engine/CollisionProfile.h"
//#include "Materials/MaterialInterface.h"
//
//AMountainGenVoxelActor::AMountainGenVoxelActor()
//{
//    PrimaryActorTick.bCanEverTick = false;
//
//    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
//    RootComponent = ProcMesh;
//
//    // 충돌 설정을 조금 더 명시적으로
//    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
//
//    // 보이는 메쉬 그대로를 콜리전으로 쓰겠다
//    ProcMesh->bUseComplexAsSimpleCollision = true;
//
//    // 일단 디버깅할 땐 비동기 끄는 걸 추천
//    ProcMesh->bUseAsyncCooking = false;
//
//    // 기본값
//    SizeX = 256;
//    SizeY = 128;
//    SizeZ = 40;
//    VoxelSize = 100.f;
//
//    SphereRadius = 10.f;
//
//    // ✅ 노이즈 기본값
//    Seed = 1337;
//    NoiseScale = 0.6f;     // 작을수록 덩어리 큼
//    NoiseAmplitude = 1.0f;  // 울퉁불퉁 정도
//
//    NoiseOctaves = 4;        // 🔥 노이즈 3겹
//    NoiseRoughness = 2.0f;   // 각 겹마다 2배씩 더 세밀하게
//
//    VoxelMaterial = nullptr;
//}
//
//void AMountainGenVoxelActor::OnConstruction(const FTransform& Transform)
//{
//    Super::OnConstruction(Transform);
//    GenerateVoxelMesh();
//
//    // 🔥 메쉬 만든 뒤 머티리얼 바인딩
//    if (VoxelMaterial && ProcMesh)
//    {
//        ProcMesh->SetMaterial(0, VoxelMaterial);
//    }
//}
//
//// 2D FBM 노이즈: 여러 옥타브 PerlinNoise2D를 섞어서 거친 패턴 생성
//static float FBM2D(
//    const FVector2D& P,
//    const FVector2D& NoiseOffset,
//    float BaseScale,
//    int32 Octaves,
//    float Roughness)
//{
//    float Total = 0.0f;
//    float Amplitude = 1.0f;
//    float Frequency = 1.0f;
//    float AmpSum = 0.0f;
//
//    Octaves = FMath::Max(1, Octaves);
//
//    for (int32 o = 0; o < Octaves; ++o)
//    {
//        const FVector2D Pos = (P + NoiseOffset) * BaseScale * Frequency;
//        float n = FMath::PerlinNoise2D(Pos); // -1 ~ 1
//
//        Total += n * Amplitude;
//        AmpSum += Amplitude;
//
//        Amplitude *= 0.5f;         // 고주파로 갈수록 영향 줄이기
//        Frequency *= Roughness;    // 주파수(세밀함) 증가
//    }
//
//    if (AmpSum > 0.0f)
//    {
//        Total /= AmpSum;           // 대략 -1 ~ 1 유지
//    }
//
//    return Total;                  // -1 ~ 1
//}
//
//
//void AMountainGenVoxelActor::GenerateVoxelMesh()
//{
//    if (!ProcMesh) return;
//
//    ProcMesh->ClearAllMeshSections();
//
//    TArray<FVector> Vertices;
//    TArray<int32> Triangles;
//    TArray<FVector> Normals;
//    TArray<FVector2D> UVs;
//    TArray<FLinearColor> Colors;
//    TArray<FProcMeshTangent> Tangents;
//
//    const int32 NumX = SizeX;
//    const int32 NumY = SizeY;
//    const float QuadSize = VoxelSize;
//
//    const float HalfX = static_cast<float>(NumX) * 0.5f;
//    const float HalfY = static_cast<float>(NumY) * 0.5f;
//
//    // 시드 기반 오프셋
//    FRandomStream Stream(Seed);
//    const FVector2D NoiseOffset(
//        Stream.FRandRange(-10000.f, 10000.f),
//        Stream.FRandRange(-10000.f, 10000.f)
//    );
//
//    // 정점 개수
//    const int32 NumVertsX = NumX + 1;
//    const int32 NumVertsY = NumY + 1;
//    const int32 NumVerts = NumVertsX * NumVertsY;
//
//    Vertices.SetNum(NumVerts);
//    UVs.SetNum(NumVerts);
//    Colors.SetNum(NumVerts);
//    Normals.SetNum(NumVerts);
//    Tangents.SetNum(NumVerts);
//
//    auto IndexOf = [NumVertsX](int32 X, int32 Y)
//        {
//            return X + Y * NumVertsX;
//        };
//
//    // 1) 정점 + 높이 생성
//    for (int32 y = 0; y < NumVertsY; ++y)
//    {
//        for (int32 x = 0; x < NumVertsX; ++x)
//        {
//            // -1 ~ 1 로컬 좌표 (산 중심)
//            const float LX = (static_cast<float>(x) - HalfX) / HalfX;  // -1 ~ 1
//            const float LY = (static_cast<float>(y) - HalfY) / HalfY;  // -1 ~ 1
//
//            // 🔥 FBM 노이즈 값 (-1 ~ 1)
//            const FVector2D P(LX, LY);
//            const float Nfbm = FBM2D(P, NoiseOffset, NoiseScale, NoiseOctaves, NoiseRoughness);  // -1~1
//
//            // 🔥 ridged noise: 봉우리 강조 (0 ~ 1)
//            float Ridge01 = 1.0f - FMath::Abs(Nfbm);   // 골짜기(0) 봉우리(1)
//            Ridge01 = FMath::Pow(Ridge01, 3.0f);      // 지수 올릴수록 봉우리 더 뾰족
//
//            // 🔥 Y 방향으로만 살짝 마스크를 줘서, 화면 중앙 쪽에 산맥이 모이게 (원하면)
//            float Band = FMath::Clamp(1.0f - FMath::Abs(LY) * 0.8f, 0.0f, 1.0f);
//            // Band 빼고 싶으면 그냥 1.0f로 두면 됨
//
//            // 최종 높이 [0,1]
//            float Height01 = Ridge01 * Band;
//
//            // 복셀 개수 기준 높이
//            float HeightVoxelsFloat = Height01 * static_cast<float>(SizeZ);
//
//            // 혹시 음수/과도한 값 막기
//            HeightVoxelsFloat = FMath::Clamp(
//                HeightVoxelsFloat,
//                0.0f,
//                static_cast<float>(SizeZ)
//            );
//
//            const float HeightZ = HeightVoxelsFloat * VoxelSize;
//
//            const int32 Index = IndexOf(x, y);
//
//            Vertices[Index] = FVector(
//                static_cast<float>(x) * QuadSize,
//                static_cast<float>(y) * QuadSize,
//                HeightZ
//            );
//
//            UVs[Index] = FVector2D(
//                static_cast<float>(x) / static_cast<float>(NumX),
//                static_cast<float>(y) / static_cast<float>(NumY)
//            );
//
//            Colors[Index] = FLinearColor::White;
//            Normals[Index] = FVector::ZeroVector;
//            Tangents[Index] = FProcMeshTangent(FVector::RightVector, false);
//        }
//    }
//
//    // 2) 인덱스(삼각형) 생성
//    for (int32 y = 0; y < NumY; ++y)
//    {
//        for (int32 x = 0; x < NumX; ++x)
//        {
//            const int32 I00 = IndexOf(x, y);
//            const int32 I10 = IndexOf(x + 1, y);
//            const int32 I01 = IndexOf(x, y + 1);
//            const int32 I11 = IndexOf(x + 1, y + 1);
//
//            // 두 개의 삼각형 (CCW)
//            Triangles.Add(I00);
//            Triangles.Add(I11);
//            Triangles.Add(I10);
//
//            Triangles.Add(I00);
//            Triangles.Add(I01);
//            Triangles.Add(I11);
//        }
//    }
//
//    // 3) 노멀 생성 (부드러운 셰이딩)
//    for (int32 i = 0; i < Triangles.Num(); i += 3)
//    {
//        const int32 IA = Triangles[i];
//        const int32 IB = Triangles[i + 1];
//        const int32 IC = Triangles[i + 2];
//
//        const FVector& A = Vertices[IA];
//        const FVector& B = Vertices[IB];
//        const FVector& C = Vertices[IC];
//
//        const FVector Edge1 = B - A;
//        const FVector Edge2 = C - A;
//        const FVector FaceNormal = FVector::CrossProduct(Edge2, Edge1).GetSafeNormal();
//
//        Normals[IA] += FaceNormal;
//        Normals[IB] += FaceNormal;
//        Normals[IC] += FaceNormal;
//    }
//
//    for (int32 i = 0; i < Normals.Num(); ++i)
//    {
//        Normals[i].Normalize();
//    }
//
//    // 4) 메쉬 + 콜리전 생성
//    ProcMesh->CreateMeshSection_LinearColor(
//        0,
//        Vertices,
//        Triangles,
//        Normals,
//        UVs,
//        Colors,
//        Tangents,
//        true,   // bCreateCollision
//        false   // bSRGBConversion
//    );
//
//    ProcMesh->bUseComplexAsSimpleCollision = true;
//
//    // 머티리얼도 있으면 적용
//    if (VoxelMaterial)
//    {
//        ProcMesh->SetMaterial(0, VoxelMaterial);
//    }
//}
//
//void AMountainGenVoxelActor::Regenerate()
//{
//    GenerateVoxelMesh();
//
//    if (VoxelMaterial)
//    {
//        ProcMesh->SetMaterial(0, VoxelMaterial);
//    }
//}
//
//void AMountainGenVoxelActor::SetSeed(int32 NewSeed)
//{
//    Seed = NewSeed;
//    Regenerate();
//}
//
//// 내부 헬퍼 함수: 복셀 하나(큐브) 추가 – 6면 전부 생성
////static void AddCube(
////    const FVector& MinCorner,
////    float CubeSize,
////    TArray<FVector>& Vertices,
////    TArray<int32>& Triangles,
////    TArray<FVector>& Normals,
////    TArray<FVector2D>& UVs,
////    TArray<FLinearColor>& Colors,
////    TArray<FProcMeshTangent>& Tangents)
////{
////    const FVector P000 = MinCorner;
////    const FVector P100 = MinCorner + FVector(CubeSize, 0.f, 0.f);
////    const FVector P010 = MinCorner + FVector(0.f, CubeSize, 0.f);
////    const FVector P110 = MinCorner + FVector(CubeSize, CubeSize, 0.f);
////    const FVector P001 = MinCorner + FVector(0.f, 0.f, CubeSize);
////    const FVector P101 = MinCorner + FVector(CubeSize, 0.f, CubeSize);
////    const FVector P011 = MinCorner + FVector(0.f, CubeSize, CubeSize);
////    const FVector P111 = MinCorner + FVector(CubeSize, CubeSize, CubeSize);
////
////    auto AddFace = [&](
////        const FVector& V0, const FVector& V1,
////        const FVector& V2, const FVector& V3,
////        const FVector& Normal)
////        {
////            const int32 IndexStart = Vertices.Num();
////
////            Vertices.Add(V0);
////            Vertices.Add(V1);
////            Vertices.Add(V2);
////            Vertices.Add(V3);
////
////            Normals.Add(Normal);
////            Normals.Add(Normal);
////            Normals.Add(Normal);
////            Normals.Add(Normal);
////
////            UVs.Add(FVector2D(0.f, 0.f));
////            UVs.Add(FVector2D(1.f, 0.f));
////            UVs.Add(FVector2D(1.f, 1.f));
////            UVs.Add(FVector2D(0.f, 1.f));
////
////            Colors.Add(FLinearColor::White);
////            Colors.Add(FLinearColor::White);
////            Colors.Add(FLinearColor::White);
////            Colors.Add(FLinearColor::White);
////
////            FVector TangentDir = FVector::CrossProduct(Normal, FVector::UpVector);
////            if (TangentDir.IsNearlyZero())
////            {
////                TangentDir = FVector::RightVector;
////            }
////            const FProcMeshTangent Tangent(TangentDir, false);
////            Tangents.Add(Tangent);
////            Tangents.Add(Tangent);
////            Tangents.Add(Tangent);
////            Tangents.Add(Tangent);
////
////            Triangles.Add(IndexStart + 0);
////            Triangles.Add(IndexStart + 1);
////            Triangles.Add(IndexStart + 2);
////            Triangles.Add(IndexStart + 0);
////            Triangles.Add(IndexStart + 2);
////            Triangles.Add(IndexStart + 3);
////        };
////
////    // +X
////    AddFace(
////        P100, P101, P111, P110,
////        FVector(1.f, 0.f, 0.f)
////    );
////
////    // -X
////    AddFace(
////        P000, P010, P011, P001,
////        FVector(-1.f, 0.f, 0.f)
////    );
////
////    // +Y
////    AddFace(
////        P010, P110, P111, P011,
////        FVector(0.f, 1.f, 0.f)
////    );
////
////    // -Y
////    AddFace(
////        P000, P001, P101, P100,
////        FVector(0.f, -1.f, 0.f)
////    );
////
////    // +Z (위)
////    AddFace(
////        P001, P011, P111, P101,
////        FVector(0.f, 0.f, 1.f)
////    );
////
////    // -Z (아래)
////    AddFace(
////        P000, P100, P110, P010,
////        FVector(0.f, 0.f, -1.f)
////    );
////}
////
////
////void AMountainGenVoxelActor::GenerateVoxelMesh()
////{
////    if (!ProcMesh) return;
////
////    ProcMesh->ClearAllMeshSections();
////
////    TArray<FVector> Vertices;
////    TArray<int32> Triangles;
////    TArray<FVector> Normals;
////    TArray<FVector2D> UVs;
////    TArray<FLinearColor> Colors;
////    TArray<FProcMeshTangent> Tangents;
////
////    const float HalfX = static_cast<float>(SizeX) * 0.5f;
////    const float HalfY = static_cast<float>(SizeY) * 0.5f;
////    const float HalfZ = static_cast<float>(SizeZ) * 0.5f;
////
////    FRandomStream Stream(Seed);
////    const FVector NoiseOffset(
////        Stream.FRandRange(-10000.f, 10000.f),
////        Stream.FRandRange(-10000.f, 10000.f),
////        Stream.FRandRange(-10000.f, 10000.f)
////    );
////
////    for (int32 x = 0; x < SizeX; ++x)
////    {
////        for (int32 y = 0; y < SizeY; ++y)
////        {
////            for (int32 z = 0; z < SizeZ; ++z)
////            {
////                const float CX = static_cast<float>(x) - HalfX + 0.5f;
////                const float CY = static_cast<float>(y) - HalfY + 0.5f;
////                const float CZ = static_cast<float>(z) - HalfZ + 0.5f;
////
////                const FVector P(CX, CY, CZ);
////                const float Dist = P.Size();
////
////                const float N = FMath::PerlinNoise3D((P + NoiseOffset) * NoiseScale);
////                const float EffectiveRadius = SphereRadius + N * NoiseAmplitude;
////
////                if (Dist <= EffectiveRadius)
////                {
////                    const FVector MinCorner(
////                        static_cast<float>(x) * VoxelSize,
////                        static_cast<float>(y) * VoxelSize,
////                        static_cast<float>(z) * VoxelSize
////                    );
////
////                    AddCube(
////                        MinCorner,
////                        VoxelSize,
////                        Vertices,
////                        Triangles,
////                        Normals,
////                        UVs,
////                        Colors,
////                        Tangents
////                    );
////                }
////            }
////        }
////    }
////
////    ProcMesh->CreateMeshSection_LinearColor(
////        0,
////        Vertices,
////        Triangles,
////        Normals,
////        UVs,
////        Colors,
////        Tangents,
////        /*bCreateCollision=*/ true,
////        /*bSRGBConversion=*/ false
////    );
////
////    ProcMesh->bUseComplexAsSimpleCollision = true;
////}