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

    // Algorithm diversity without building a full node graph yet.
    // The terrain module can swap density styles while still using the same goal-driven scoring loop.
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

    // Preset-derived detail controls.
    // Not exposed to Details panel: Difficulty preset owns these values.
    // Macro shape strength and small surface detail are intentionally separated in code.
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
    // 절벽 생성이 끝난 뒤, 최상단에 별도 평지 컴포넌트를 추가한다.
    // 이 평지는 절벽 메시/목표 지표/Surface Metadata에는 포함되지 않는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau")
    bool bAddTopFlatPlateau = true;

    // 상단 평지가 절벽 뒤쪽으로 이어지는 깊이. Actor 기준 -X 방향으로 뻗는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "0.0"))
    float TopPlateauDepthCm = 24000.f;

    // 상단 평지 블록의 기본 두께. 기본값 100cm = 1m.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "10.0"))
    float TopPlateauThicknessCm = 100.f;

    // 절벽과 맞닿는 앞쪽 경계를 절벽 상단 노이즈/실루엣에 맞출지 여부.
    // true면 접합부만 절벽 모양을 따라가고, 뒤쪽은 단순 박스 형태를 유지한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau")
    bool bConformTopPlateauToCliff = true;

    // 접합부를 절벽 앞면 방향으로 얼마나 겹치게 할지 결정한다.
    // 너무 크면 절벽 위를 과하게 덮고, 0이면 절벽 실루엣에 맞춘 위치에서 끝난다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "0.0"))
    float TopPlateauFrontOverlapCm = 0.f;

    // 평지 상단 높이 보정. 기본 0이면 BaseHeightCm + CliffHeightCm 위치가 평지 윗면이다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau")
    float TopPlateauHeightOffsetCm = 0.f;

    // 절벽 실루엣을 따라가는 앞쪽 경계의 Y 방향 분할 수.
    // 값이 클수록 절벽 상단 모양을 더 촘촘하게 따라가지만 정점 수가 증가한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "4", ClampMax = "512"))
    int32 TopPlateauConformSegmentsY = 96;

    // 각 Y 지점에서 절벽 상단 후보를 찾을 때 사용하는 좌우 검색 폭.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "10.0"))
    float TopPlateauConformSampleBandCm = 800.f;

    // 평지 윗면 기준 아래쪽으로 어느 깊이까지 절벽 상단 후보를 찾을지 결정한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "100.0"))
    float TopPlateauConformSearchDepthCm = 6000.f;

    // 접합부가 절벽 표면과 미세하게 벌어지는 것을 줄이기 위해 아래쪽으로 살짝 내리는 값.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "0.0"))
    float TopPlateauContactOverlapDownCm = 20.f;

    // 절벽 상단 후보가 너무 낮게 잡혔을 때 앞쪽 접합부가 과도하게 아래로 내려가는 것을 제한한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|TopPlateau", meta = (ClampMin = "100.0"))
    float TopPlateauMaxConformDropCm = 2500.f;

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