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
    // 6-1) Plateau Terrain Module
    // ========================================================
    // 절벽 생성이 끝난 뒤, 최상단에 별도 Plateau 지형 모듈을 생성한다.
    // Plateau는 절벽 Metrics / Surface Metadata / Placement Query에는 포함되지 않는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule")
    bool bAddTopFlatPlateau = true;

    // Plateau가 절벽 뒤쪽으로 이어지는 깊이. Actor 기준 -X 방향으로 뻗는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "0.0"))
    float TopPlateauDepthCm = 24000.f;

    // Plateau 하부 두께. 기본값 20cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "10.0"))
    float TopPlateauThicknessCm = 20.f;

    // 접합부에서 Plateau가 절벽 상단 실루엣을 따라가도록 한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule")
    bool bConformTopPlateauToCliff = true;

    // 절벽 앞면 방향으로 겹치는 길이. 접합부의 미세 틈을 덮고 싶을 때만 올린다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "0.0"))
    float TopPlateauFrontOverlapCm = 0.f;

    // Plateau 기준 상단 높이 보정.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule")
    float TopPlateauHeightOffsetCm = 0.f;

    // 접합부를 Y방향으로 나누는 수. 값이 높을수록 절벽 실루엣을 더 촘촘하게 따른다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "4", ClampMax = "512"))
    int32 TopPlateauConformSegmentsY = 128;

    // 접합부 샘플링 시 같은 Y로 인정할 폭.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "10.0"))
    float TopPlateauConformSampleBandCm = 800.f;

    // 접합부를 찾기 위해 절벽 상단 아래로 탐색할 깊이.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "100.0"))
    float TopPlateauConformSearchDepthCm = 6000.f;

    // 접합부 앞면을 절벽 상단보다 살짝 아래로 내려 겹치게 하는 값.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "0.0"))
    float TopPlateauContactOverlapDownCm = 20.f;

    // 접합부가 기준 Plateau 높이에서 최대로 내려갈 수 있는 거리.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "100.0"))
    float TopPlateauMaxConformDropCm = 2500.f;

    // Plateau 전용 Seed. 최종 Cliff Seed + 이 값을 사용해 자연스러운 상단 지형 노이즈를 만든다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule")
    int32 PlateauSeedOffset = 100000;

    // Plateau 깊이 방향 격자 분할 수. 공유 정점 기반 Heightfield로 생성된다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "1", ClampMax = "256"))
    int32 PlateauSegmentsX = 96;

    // Plateau 윗면의 낮은 언덕 높이 강도. 노이즈는 기준 평면 아래로 내려가지 않고 위로만 솟는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "0.0"))
    float PlateauSurfaceNoiseStrengthCm = 2600.f;

    // Plateau 윗면 노이즈 스케일. 값이 클수록 넓고 완만한 언덕이 된다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "100.0"))
    float PlateauSurfaceNoiseScaleCm = 24000.f;

    // Plateau 윗면 노이즈 옥타브 수. 높을수록 작은 요철이 늘어난다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "1", ClampMax = "8"))
    int32 PlateauSurfaceNoiseOctaves = 5;

    // Plateau 외곽과 절벽 접합부에서 노이즈를 서서히 줄이는 거리.
    // 가장자리와 접합부가 과하게 들뜨거나 찢어져 보이는 것을 막는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "0.0"))
    float PlateauSurfaceEdgeFadeCm = 1200.f;

    // Plateau 머티리얼 UV 타일링 기준 크기.
    // 값이 클수록 텍스처 반복이 덜 보이지만, 너무 크면 텍스처가 늘어나 보일 수 있다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "100.0"))
    float PlateauMaterialUVScaleCm = 18000.f;

    // Plateau 머티리얼 UV를 회전시켜 지형 축과 텍스처 반복 방향이 딱 맞물리는 것을 줄인다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule")
    float PlateauMaterialUVRotationDeg = 17.f;

    // Plateau UV에 약한 저주파 왜곡을 넣어 반복 패턴을 줄인다.
    // 0이면 UV 왜곡 없음.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PlateauMaterialUVWarpStrength = 0.08f;

    // Plateau UV 왜곡의 스케일.
    // 값이 클수록 넓고 느린 왜곡, 작을수록 촘촘한 왜곡이 된다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|PlateauModule", meta = (ClampMin = "100.0"))
    float PlateauMaterialUVWarpScaleCm = 24000.f;

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