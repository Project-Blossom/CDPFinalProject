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
struct FMGTargets
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CaveMin = 0.00f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CaveMax = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OverhangMin = 0.00f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OverhangMax = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SteepMin = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SteepMax = 0.20f;
};

USTRUCT(BlueprintType)
struct FMountainGenSettings
{
    GENERATED_BODY()

    // ============================================================
    // 0) Reproducibility
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Main")
    int32 Seed = -1;

    // ============================================================
    // 1) Fixed inputs
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Main")
    float BaseHeightCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Main", meta = (ClampMin = "1000.0"))
    float HeightAmpCm = 30000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Main", meta = (ClampMin = "1000.0"))
    float EnvelopeRadiusCm = 32000.f;

    // ============================================================
    // 2) Chunk / sampling
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkX = 128;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkY = 128;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "8", ClampMax = "512"))
    int32 ChunkZ = 128;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Chunk", meta = (ClampMin = "1.0"))
    float VoxelSizeCm = 200.f;

    // ============================================================
    // 3) Meshing
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Meshing")
    float IsoLevel = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Meshing")
    bool bCreateCollision = true;

    // ============================================================
    // 3.5) Front-face only
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|FrontFace")
    bool bFrontFaceOnly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|FrontFace", meta = (ClampMin = "0.0"))
    float FrontBandDepthCm = 0.0f;

    // ============================================================
    // 4) Difficulty / AutoTune
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    EMountainGenDifficulty Difficulty = EMountainGenDifficulty::Easy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    bool bAutoTune = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "1", ClampMax = "20"))
    int32 AutoTuneMaxIters = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "1", ClampMax = "2000"))
    int32 SeedSearchTries = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    bool bRetrySeedUntilSatisfied = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "1", ClampMax = "2000"))
    int32 MaxSeedAttempts = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float MetricsStepCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    FMGTargets Targets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SteepDotThreshold = 0.309f;

    // ============================================================
    // 5) Advanced
    // ============================================================

    // ---------------- Envelope shaping ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Envelope", meta = (ClampMin = "100.0", AdvancedDisplay))
    float EnvelopeEdgeFadeCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Envelope", meta = (ClampMin = "0.0", AdvancedDisplay))
    float EnvelopeEdgeCutCm = 30000.f;

    // ---------------- Base 3D field ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Field3D", meta = (ClampMin = "0.0", AdvancedDisplay))
    float BaseField3DStrengthCm = 12000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Field3D", meta = (ClampMin = "100.0", AdvancedDisplay))
    float BaseField3DScaleCm = 16000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Field3D", meta = (ClampMin = "1", ClampMax = "8", AdvancedDisplay))
    int32 BaseField3DOctaves = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Field3D", meta = (ClampMin = "1.0", ClampMax = "4.0", AdvancedDisplay))
    float BaseField3DRidgedPower = 2.0f;

    // ---------------- Detail ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Detail", meta = (ClampMin = "300.0", AdvancedDisplay))
    float DetailScaleCm = 6000.f;

    // ---------------- Gravity shaping ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Gravity", meta = (ClampMin = "0.01", AdvancedDisplay))
    float GravityStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Gravity", meta = (ClampMin = "0.1", AdvancedDisplay))
    float GravityScale = 2.f;

    // ---------------- Warp ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Warp", meta = (ClampMin = "1.0", AdvancedDisplay))
    float WarpPatchCm = 15000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Warp", meta = (ClampMin = "0.0", AdvancedDisplay))
    float WarpAmpCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Warp", meta = (ClampMin = "0.0", AdvancedDisplay))
    float WarpStrength = 1.0f;

    // ---------------- Overhang ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Overhang", meta = (ClampMin = "1.0", AdvancedDisplay))
    float OverhangScaleCm = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Overhang", meta = (ClampMin = "0.0", AdvancedDisplay))
    float VolumeStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Overhang", meta = (ClampMin = "0.0", AdvancedDisplay))
    float OverhangFadeCm = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Overhang", meta = (ClampMin = "0.0", ClampMax = "1.0", AdvancedDisplay))
    float OverhangBias = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Overhang", meta = (ClampMin = "0.0", AdvancedDisplay))
    float OverhangDepthCm = 2500.f;

    // ---------------- Caves ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", AdvancedDisplay))
    float CaveStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "1.0", AdvancedDisplay))
    float CaveScaleCm = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", AdvancedDisplay))
    float CaveMinHeightCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", AdvancedDisplay))
    float CaveMaxHeightCm = 22000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", ClampMax = "1.0", AdvancedDisplay))
    float CaveThreshold = 0.62f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", ClampMax = "1.0", AdvancedDisplay))
    float CaveBand = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", AdvancedDisplay))
    float CaveDepthCm = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cave", meta = (ClampMin = "0.0", AdvancedDisplay))
    float CaveNearSurfaceCm = 8000.0f;

    // ---------------- switches ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Safety", meta = (AdvancedDisplay))
    bool bOverhangBiasIncreaseWhenValueIncreases = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Safety", meta = (AdvancedDisplay))
    bool bCaveHeightsAreAbsoluteWorldZ = false;

    // ---------------- Cliff (detail band) ----------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cliff", meta = (ClampMin = "100.0", AdvancedDisplay))
    float CliffDepthCm = 18000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Advanced|Cliff", meta = (ClampMin = "100.0", AdvancedDisplay))
    float CliffSurfaceBandCm = 2500.f;

    // ============================================================
    // Cliff Base
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff")
    bool bUseCliffBase = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "1000.0"))
    float CliffHalfWidthCm = 30000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "1000.0"))
    float CliffHeightCm = 35000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "500.0"))
    float CliffThicknessCm = 20000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "0.0"))
    float CliffSurfaceAmpCm = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "300.0"))
    float CliffSurfaceScaleCm = 7000.f;

    // ============================================================
    // Deterministic voxel post-process caves
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost")
    bool bEnableCaves = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "100.0"))
    float CaveTileSizeCm = 12800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "10.0"))
    float CaveDiameterCm = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "0", ClampMax = "6"))
    int32 CaveMinSolidNeighbors = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "0", ClampMax = "64"))
    int32 CavesPerTile_Easy = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "0", ClampMax = "64"))
    int32 CavesPerTile_Normal = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "0", ClampMax = "64"))
    int32 CavesPerTile_Hard = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|CavePost", meta = (ClampMin = "0", ClampMax = "64"))
    int32 CavesPerTile_Extreme = 8;
};