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

UENUM(BlueprintType)
enum class EMGTerrainAlgorithm : uint8
{
    RidgedCliff       UMETA(DisplayName = "Ridged Cliff"),
    DensityFBM        UMETA(DisplayName = "Density FBM"),
    LayeredNoise      UMETA(DisplayName = "Layered Noise"),
    ZoneMaskedDensity UMETA(DisplayName = "Zone Masked Density")
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

    float RoughnessMax = 0.18f;
    float ShadowRiskMax = 0.08f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Terrain")
    EMGTerrainAlgorithm TerrainAlgorithm = EMGTerrainAlgorithm::RidgedCliff;

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

    float DetailStrengthCm = 0.f;
    float SurfaceRoughnessStrengthCm = 0.f;
    float SurfaceRoughnessMaskStrength = 0.75f;
    float SurfaceQualityScoreWeight = 1.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Cliff", meta = (ClampMin = "0.0"))
    float FrontBandDepthCm = 0.f;

    // ========================================================
    // 6-1) Top Flat Plateau
    // ========================================================
    // 절벽 생성이 끝난 뒤, 최상단에 단순 직육면체 평지 블록을 추가한다.
    // 왼쪽에서 봤을 때 절벽 위에 평평한 블록이 얹힌 ㄱ자 형태를 만든다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau")
    bool bAddTopFlatPlateau = true;

    // 상단 평지 블록이 절벽 뒤쪽으로 이어지는 깊이. Actor 기준 -X 방향으로 뻗는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "0.0"))
    float TopPlateauDepthCm = 24000.f;

    // 상단 평지 블록의 두께. 기본값 100cm = 1m.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "10.0"))
    float TopPlateauThicknessCm = 100.f;

    // 절벽 앞면 방향으로 겹치는 길이. 기본값 0이면 Actor X=0에서 정확히 끝나는 단순 블록이다.
    // 틈을 덮고 싶을 때만 값을 올린다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "0.0"))
    float TopPlateauFrontOverlapCm = 0.f;

    // 평지 상단 높이 보정. 기본 0이면 BaseHeightCm + CliffHeightCm 위치가 블록 윗면이다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau")
    float TopPlateauHeightOffsetCm = 0.f;

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

    // ========================================================
    // 9) Goal-Driven Hierarchical Search
    // ========================================================
    // 목표 기반 탐색을 메시 생성 전에 저비용 Proxy Metrics로 먼저 거르는 방식으로 수행한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch")
    bool bUseHierarchicalGoalSearch = true;

    // Proxy 단계에서 검사할 후보 수. 값이 클수록 목표에 맞는 Seed를 찾을 가능성은 높지만 CPU 비용이 증가한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch", meta = (ClampMin = "1", ClampMax = "4096"))
    int32 ProxySeedBudget = 128;

    // Proxy 단계 통과 후 정밀 Metrics로 다시 평가할 상위 후보 수.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch", meta = (ClampMin = "1", ClampMax = "128"))
    int32 ProxySurvivorCount = 12;

    // Proxy Metrics에 사용할 저해상도 샘플 수. 전체 메시 생성 없이 목표 가능성을 빠르게 추정한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch", meta = (ClampMin = "16", ClampMax = "2048"))
    int32 ProxyMetricsSamplesPerTry = 32;

    // 목표 오차를 보고 다음 후보 그룹의 생성 파라미터를 보정하는 횟수.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch", meta = (ClampMin = "1", ClampMax = "16"))
    int32 GoalFeedbackRounds = 3;

    // 한 Feedback Round에서 평가할 Proxy 후보 수. 0이면 ProxySeedBudget / GoalFeedbackRounds로 자동 분배한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch", meta = (ClampMin = "0", ClampMax = "1024"))
    int32 GoalFeedbackBatchSize = 0;

    // 필수 품질 조건 위반 후보에 부여하는 벌점 배율.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|GoalSearch", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float HardConstraintPenaltyWeight = 12.0f;

};