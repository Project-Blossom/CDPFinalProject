#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MountainGenVoxelActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class MOUNTAINGEN_API AMountainGenVoxelActor : public AActor
{
    GENERATED_BODY()

public:
    AMountainGenVoxelActor();

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void Regenerate();

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void SetSeed(int32 NewSeed);

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UProceduralMeshComponent* ProcMesh;

    // =========================
    // Chunk Size (복셀 개수)
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkX = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkY = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkZ = 64;

    // 복셀 크기(cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "1.0"))
    float VoxelSize = 100.f;

    // =========================
    // Seed
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    int32 Seed = 1557;

    // =========================
    // 3D Density Params 
    // =========================
    // 큰 형태(산 덩어리) 주파수 (작을수록 큰 덩어리)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.000001"))
    float WorldFreq = 0.002f;

    // 디테일/오버행 주파수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.000001"))
    float DetailFreq = 0.01f;

    // 동굴 주파수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.000001"))
    float CaveFreq = 0.02f;

    // 아래쪽이 solid(-), 위쪽이 air(+)가 되게 만드는 “바닥 경향”
    // 값이 너무 크면 위가 다 공기(+), 너무 작으면 전부 바위(-)가 되기 쉬움
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    float GroundSlope = 0.01f;

    // 전체 밀도 바이어스(산이 너무 공기/바위로 치우치면 이걸로 이동)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    float BaseBias = 0.0f;

    // 오버행/거칠기 강도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    float OverhangAmp = 0.5f;

    // 동굴 카빙 강도(공기쪽으로 미는 힘)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    float CaveAmp = 1.0f;

    // 동굴 생성 임계치(0~1) : 높을수록 동굴이 드물어짐
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CaveThreshold = 0.6f;

    // =========================
    // Material
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material")
    UMaterialInterface* VoxelMaterial = nullptr;

private:
    void BuildChunkAndMesh();
};