#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.generated.h"

UENUM(BlueprintType)
enum class EMountainGenDifficulty : uint8
{
    Easy     UMETA(DisplayName = "Easy"),
    Normal   UMETA(DisplayName = "Normal"),
    Hard     UMETA(DisplayName = "Hard"),
    Extreme  UMETA(DisplayName = "Extreme"),
};

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

    // ===== Difficulty / AutoTune =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Difficulty")
    EMountainGenDifficulty Difficulty = EMountainGenDifficulty::Easy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Difficulty")
    bool bAutoTune = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Difficulty", meta = (ClampMin = "1", ClampMax = "20"))
    int32 AutoTuneMaxIters = 6;

    // ===== Terrain Params =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float WorldScaleCm = 24000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Noise")
    float DetailScaleCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Volume")
    float OverhangScaleCm = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Height")
    float BaseHeightCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Height")
    float HeightAmpCm = 30000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SteepnessDetailFactor = 0.18f;

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
    float OverhangFadeCm = 15000.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave", meta = (ClampMin = "0.0"))
    float CaveDepthCm = 1500.0f; // 15m 기본

    // 동굴이 표면 근처에서만 생기게 함
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cave", meta = (ClampMin = "0.0"))
    float CaveNearSurfaceCm = 8000.0f; // 80m 기본

    // Misc
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Misc")
    float GravityStrength = 1.0f;

    // ----- Safety switches -----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Difficulty")
    bool bOverhangBiasIncreaseWhenValueIncreases = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Difficulty")
    bool bCaveHeightsAreAbsoluteWorldZ = false;
};