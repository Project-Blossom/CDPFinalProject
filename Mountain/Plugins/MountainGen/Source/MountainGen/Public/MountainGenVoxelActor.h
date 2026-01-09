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
 // 3D Density Params (Scale Cm)
 // =========================

 // 큰 형태(산 덩어리/절벽 패치) 크기(cm) - 값이 클수록 더 큰 덩어리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "100.0"))
    float WorldScaleCm = 20000.0f; // 200m

    // 중간 디테일 크기(cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "100.0"))
    float DetailScaleCm = 3000.0f; // 30m

    // 동굴 덩어리 크기(cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "100.0"))
    float CaveScaleCm = 1800.0f; // 18m

    // (선택) 높이 스케일을 Actor에서 조절하고 싶으면 노출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.0"))
    float HeightAmp = 3000.0f;

    // 오버행 강도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.0"))
    float OverhangAmp = 0.6f;

    // 동굴 강도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.0"))
    float CaveAmp = 1.0f;

    // 동굴 임계값(0~1) : 높을수록 동굴이 드물어짐
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CaveThreshold = 0.55f;

    // 전체 밀도 바이어스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    float BaseBias = 10.0f;

    // =========================
    // Material
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material")
    UMaterialInterface* VoxelMaterial = nullptr;

private:
    void BuildChunkAndMesh();
};