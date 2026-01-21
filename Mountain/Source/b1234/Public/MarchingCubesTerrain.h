#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MarchingCubesTerrain.generated.h"


class UProceduralMeshComponent;

UCLASS()
class B1234_API AMarchingCubesTerrain : public AActor
{
    GENERATED_BODY()

public:
    AMarchingCubesTerrain();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProcMesh = nullptr;

    void CreateTestTriangle();

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Grid")
    int32 NumX = 64;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Grid")
    int32 NumY = 64;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Grid")
    int32 NumZ = 64;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Grid")
    float CellSize = 100.f;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Field")
    float IsoLevel = 0.f;

    // 밀도 샘플 (NumX * NumY * NumZ)
    UPROPERTY()
    TArray<float> Density;

    FORCEINLINE int32 Index(int32 X, int32 Y, int32 Z) const
    {
        return X + NumX * (Y + NumY * Z);
    }

    FORCEINLINE FVector GridToWorld(int32 X, int32 Y, int32 Z) const
    {
        // 액터 로컬 좌표 기준 (나중에 청크면 여기에 오프셋 추가)
        return FVector(X * CellSize, Y * CellSize, Z * CellSize);
    }

    void AllocateDensity();
    void BuildDensityField_TestPlane();
    void LogDensityRange() const;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug")
    bool bDebugDrawSlice = true;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug", meta = (ClampMin = "0", ClampMax = "1024"))
    int32 DebugSliceZ = 20; // Z 슬라이스 인덱스 (0 ~ NumZ-1)

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug")
    float DebugIsoEpsilon = 30.f;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug")
    float DebugPointSize = 10.f;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug")
    float DebugLifeTime = 5.f;

    void DebugDrawDensitySlice() const;

    // ===== Step3: Rock/Cliff Params =====
    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float BaseHeightRatio = 0.35f; // 전체 높이 중 바닥 기준 (0~1)

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float CliffStrength = 0.9f; // 절벽 강도(0~1 정도)

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float CliffFreq = 0.0025f; // 절벽/능선 스케일(작을수록 큰 지형)

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float DetailFreq = 0.02f; // 표면 디테일 노이즈

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float DetailAmp = 80.f; // 디테일 높이(센치)

    // 3D 오버행(동굴/튀어나옴)용
    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float OverhangFreq = 0.015f;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    float OverhangAmp = 120.f;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Shape")
    bool bEnableOverhang = true;

    void BuildDensityField_CliffRock();
    float HeightFunc_Cliff(const FVector& P) const;
    float Noise2D(float X, float Y, float Freq) const;
    float Noise3D(float X, float Y, float Z, float Freq) const;

    void DebugDrawHeightPoints() const;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug")
    bool bDebugDrawHeight = true;

    UPROPERTY(EditAnywhere, Category = "MarchingCubes|Debug")
    int32 DebugHeightStep = 2; // 1이면 촘촘, 2~4 추천

    bool TriangulateCell(int32 X, int32 Y, int32 Z,
        TArray<FVector>& OutVerts,
        TArray<int32>& OutTris) const;

    FORCEINLINE float Sample(int32 X, int32 Y, int32 Z) const
    {
        return Density[Index(X, Y, Z)];
    }

    FVector VertexInterp(float Iso, const FVector& P1, const FVector& P2, float V1, float V2) const;
};
