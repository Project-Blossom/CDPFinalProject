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
    // Chunk
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkX = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkY = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkZ = 256;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "1.0"))
    float VoxelSize = 100.f;

    // Meshing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Meshing")
    float IsoLevel = 0.0f;

    // Seed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density")
    int32 Seed = 1557;

    // =========================
    // Density Params (Generator)
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "100.0"))
    float WorldScaleCm = 24000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density", meta = (ClampMin = "100.0"))
    float DetailScaleCm = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Height")
    float BaseHeightCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Height", meta = (ClampMin = "1.0"))
    float HeightAmpCm = 30000.0f;

    // 끝으로 갈수록 높게 (Peak 느낌)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Height")
    float RampHeightCm = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Height", meta = (ClampMin = "1.0"))
    float RampLengthCm = 6400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Volume", meta = (ClampMin = "0.0"))
    float VolumeStrength = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Volume", meta = (ClampMin = "1.0"))
    float OverhangFadeCm = 3000.0f;

    // Warp
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Warp", meta = (ClampMin = "100.0"))
    float WarpPatchCm = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Warp", meta = (ClampMin = "0.0"))
    float WarpAmpCm = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Warp", meta = (ClampMin = "0.0"))
    float WarpStrength = 1.0f;

    // Caves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Caves", meta = (ClampMin = "100.0"))
    float CaveScaleCm = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Caves", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float CaveThreshold = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Caves", meta = (ClampMin = "0.0"))
    float CaveStrength = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Density|Caves", meta = (ClampMin = "0.01"))
    float CaveBand = 0.25f;

    // =========================
    // Postprocess (Mask)
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Post")
    bool bRemoveIslands = true;

    // 바닥 z=0~GroundBandZ-1 을 "땅"으로 보고 연결된 solid만 유지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Post", meta = (ClampMin = "1", ClampMax = "16"))
    int32 GroundBandZ = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Post")
    bool bUseClosing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Post", meta = (ClampMin = "0", ClampMax = "8"))
    int32 ClosingDilateIters = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Post", meta = (ClampMin = "0", ClampMax = "8"))
    int32 ClosingErodeIters = 1;

    // ★ 계단 방지 핵심: 마스크 결과를 density에 "살짝"만 반영
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Post", meta = (ClampMin = "0.0"))
    float SoftPushCm = 200.0f;

    // Material
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material")
    UMaterialInterface* VoxelMaterial = nullptr;

private:
    void BuildChunkAndMesh();
};