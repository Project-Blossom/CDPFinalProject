#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.generated.h"

USTRUCT(BlueprintType)
struct FMountainGenSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Seed")
    int32 Seed = 1557;

    // ===== Chunk =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkX = 128;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkY = 128;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkZ = 256;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "1.0"))
    float VoxelSizeCm = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Meshing")
    float IsoLevel = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Meshing")
    bool bCreateCollision = true;

    // ===== Terrain Params =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float WorldScaleCm = 24000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float DetailScaleCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Height")
    float BaseHeightCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Height")
    float HeightAmpCm = 30000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Ramp")
    float RampLengthCm = 60000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Ramp")
    float RampHeightCm = 45000.f;

    // Warp
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Warp")
    float WarpPatchCm = 15000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Warp")
    float WarpAmpCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Warp")
    float WarpStrength = 1.0f;

    // Volume/Overhang
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Volume")
    float VolumeStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Volume")
    float OverhangFadeCm = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Volume")
    float OverhangBias = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Volume")
    float OverhangDepthCm = 2500.f;

    // Cave
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave")
    float CaveStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave")
    float CaveScaleCm = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave")
    float CaveMinHeightCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave")
    float CaveMaxHeightCm = 22000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave")
    float CaveThreshold = 0.62f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave")
    float CaveBand = 0.12f;

    // Misc
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Misc")
    float GravityStrength = 1.0f;
};