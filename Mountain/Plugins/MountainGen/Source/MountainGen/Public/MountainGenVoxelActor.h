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

    // ===== Chunk Size (복셀 개수) =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkX = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkY = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "4", ClampMax = "256"))
    int32 ChunkZ = 64;

    // 복셀 크기(cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "1.0"))
    float VoxelSize = 100.f;

    // ===== Noise Params =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    int32 Seed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise", meta = (ClampMin = "0.0001"))
    float HeightScale = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float HeightAmp = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise", meta = (ClampMin = "0.0001"))
    float CaveScale = 0.06f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float CaveStrength = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float BaseFloor = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material")
    UMaterialInterface* VoxelMaterial = nullptr;

private:
    void BuildChunkAndMesh();
};


//#pragma once
//
//#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//#include "MountainGenVoxelActor.generated.h"
//
//class UProceduralMeshComponent;
//class UMaterialInterface;
//
//UCLASS()
//class MOUNTAINGEN_API AMountainGenVoxelActor : public AActor
//{
//    GENERATED_BODY()
//
//public:
//    AMountainGenVoxelActor();
//
//    UFUNCTION(BlueprintCallable, Category = "MountainGen")
//    void Regenerate();
//
//    UFUNCTION(BlueprintCallable, Category = "MountainGen")
//    void SetSeed(int32 NewSeed);
//
//protected:
//    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
//    UProceduralMeshComponent* ProcMesh;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
//    int32 SizeX;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
//    int32 SizeY;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
//    int32 SizeZ;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
//    float VoxelSize;
//
//    // 기준(기본) 반지름
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voxel")
//    float SphereRadius;
//
//    // ---------------------------
//    // Noise Parameters
//    // ---------------------------
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
//    int32 Seed;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (ClampMin = "0.0001"))
//    float NoiseScale;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
//    float NoiseAmplitude;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (ClampMin = "1", ClampMax = "8"))
//    int32 NoiseOctaves;
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (ClampMin = "1.0"))
//    float NoiseRoughness;
//
//    // ---------------------------
//    // Material
//    // ---------------------------
//
//    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
//    UMaterialInterface* VoxelMaterial;
//
//protected:
//    virtual void OnConstruction(const FTransform& Transform) override;
//    void GenerateVoxelMesh();
//};