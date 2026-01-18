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

};
