#pragma once
#include "CoreMinimal.h"
#include "MountainGenSettings.generated.h"

UENUM(BlueprintType)
enum class EMountainGenDifficulty : uint8
{
    Easy     UMETA(DisplayName = "Easy"),
    Normal   UMETA(DisplayName = "Normal"),
    Hard     UMETA(DisplayName = "Hard"),
};

// ============================================================
// Metrics Targets
// ============================================================
USTRUCT(BlueprintType)
struct FMGTargets
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OverhangMin = 0.00f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OverhangMax = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SteepMin = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Targets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SteepMax = 0.50f;
};

// ============================================================
// Main Settings
// ============================================================
USTRUCT(BlueprintType)
struct FMountainGenSettings
{
    GENERATED_BODY()

    // ========================================================
    // 0) Reproducibility
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Main")
    int32 Seed = -1;

    // ========================================================
    // 1) Difficulty / AutoTune
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Difficulty")
    EMountainGenDifficulty Difficulty = EMountainGenDifficulty::Easy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    bool bAutoTune = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "0", ClampMax = "30"))
    int32 FeedbackIters = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "1", ClampMax = "2000"))
    int32 SeedSearchTries = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    bool bRetrySeedUntilSatisfied = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "1", ClampMax = "2000"))
    int32 MaxSeedAttempts = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune")
    FMGTargets Targets;

    // ========================================================
    // 2) Voxel
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Voxel", meta = (ClampMin = "1.0"))
    float VoxelSizeCm = 200.f;

    // ========================================================
    // 3) Base
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Base")
    float BaseHeightCm = 0.f;

    // ========================================================
    // 4) Density Field
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Field3D")
    float BaseField3DStrengthCm = 18000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Field3D")
    float BaseField3DScaleCm = 18000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Field3D", meta = (ClampMin = "1", ClampMax = "12"))
    int32 BaseField3DOctaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Detail")
    float DetailScaleCm = 6000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Detail", meta = (ClampMin = "1", ClampMax = "12"))
    int32 DetailOctaves = 2;

    // ========================================================
    // 5) Overhang / Undercut
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Overhang")
    float VolumeStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Overhang")
    float OverhangScaleCm = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Overhang", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OverhangBias = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Overhang", meta = (ClampMin = "0.0"))
    float OverhangDepthCm = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Overhang", meta = (ClampMin = "0.0"))
    float OverhangFadeCm = 15000.f;

    // ========================================================
    // 6) Cliff
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "1000.0"))
    float CliffHalfWidthCm = 40000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "1000.0"))
    float CliffHeightCm = 80000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "200.0"))
    float CliffThicknessCm = 20000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "1000.0"))
    float CliffDepthCm = 18000.f;

    // 0 이하이면 CliffDepthCm 기반 기본 앞면 판정 범위를 사용한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "0.0"))
    float FrontBandDepthCm = 0.f;

    // ========================================================
    // 7) Meshing
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Mesh")
    float IsoLevel = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Mesh")
    bool bCreateCollision = true;

    // ========================================================
    // 8) Metrics
    // ========================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Metrics", meta = (ClampMin = "0.0"))
    float MetricsStepCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Metrics", meta = (ClampMin = "16", ClampMax = "4096"))
    int32 MetricsSamplesPerTry = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Metrics", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float SteepDotOverride = -1.f;
};
