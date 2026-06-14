#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/RandomStream.h"
#include "MonsterSpawner.generated.h"

class AMonsterBase;
class AWallCrawler;
class AFlyingPlatform;
class AFlyingAttacker;
class AHallucinationGhost;
class AMountainGenWorldActor;

UENUM()
enum class EMonsterSpawnKind : uint8
{
    WallCrawler,
    FlyingPlatform,
    FlyingAttacker
};

USTRUCT()
struct FSpawnProbeResult
{
    GENERATED_BODY()

    bool bSuccess = false;
    FVector Location = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    FVector SurfacePoint = FVector::ZeroVector;
    FVector SurfaceNormal = FVector::UpVector;
};

UENUM()
enum class ESpawnFailReason : uint8
{
    None = 0,
    NoClass,
    NoSurfaceSample,
    OutsideFrontBand,
    SameTypeDistance,
    InsideRock,
    HeightTraceFailed,
    HeightOutOfRange,
    ClearanceFailed,
    TerritoryFailed,
    FacingTraceFailed,
    FacingNormalInvalid,
    SpawnActorFailed,
    OverlapBlocked,
    Count
};

USTRUCT()
struct FSpawnFailStats
{
    GENERATED_BODY()

    int32 Counts[(int32)ESpawnFailReason::Count] = { 0 };

    void Add(ESpawnFailReason Reason)
    {
        Counts[(int32)Reason]++;
    }

    int32 Get(ESpawnFailReason Reason) const
    {
        return Counts[(int32)Reason];
    }
};

UCLASS()
class PROTOTYPE_API AMonsterSpawner : public AActor
{
    GENERATED_BODY()

public:
    AMonsterSpawner();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

public:
    // =====================================================
    // Mountain Binding
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    TObjectPtr<AMountainGenWorldActor> TargetMountain = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bAutoFindMountain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bSpawnOnlyAfterMountainGenerated = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bRespawnWhenMountainRegenerates = true;

    // =====================================================
    // Spawn Counts
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Counts", meta = (ClampMin = "0"))
    int32 WallCrawlerCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Counts", meta = (ClampMin = "0"))
    int32 FlyingPlatformCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Counts", meta = (ClampMin = "0"))
    int32 FlyingAttackerCount = 4;

    // =====================================================
    // Seeded Placement
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeded Placement")
    int32 PlacementSeed = 12345;

    // PlacementSeed가 실행 중 변경되면 기존 몬스터를 제거하고 다시 스폰한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeded Placement")
    bool bAutoRespawnWhenPlacementSeedChanges = true;

    // 아이템 스포너처럼 재스폰 전에 기존 몬스터를 먼저 제거한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeded Placement")
    bool bClearExistingMonstersBeforeSpawn = true;

    // =====================================================
    // Same-Type Distribution Rules
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
    float WallCrawlerMinDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
    float FlyingPlatformMinDistance = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
    float FlyingAttackerMinDistance = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "8", ClampMax = "512"))
    int32 FrontBandCandidateTries = 96;

    // =====================================================
    // General Rules
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerWallCrawler = 300;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerFlyingPlatform = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerFlyingAttacker = 600;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0"))
    float BoundsPadding = 1200.0f;

    // 절벽 좌우 끝에서 이 거리만큼 안쪽 구간만 스폰 후보로 사용한다.
    // 예: 50이면 Bounds.Min.Y + 50 ~ Bounds.Max.Y - 50 구간.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0"))
    float SideSpawnMarginCm = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0"))
    float FrontSpawnDepthOverrideCm = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FrontFacingNormalDotMin = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    bool bRequireCliffProximityForFlying = true;

    // =====================================================
    // Monster Classes
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Classes")
    TSubclassOf<AWallCrawler> WallCrawlerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Classes")
    TSubclassOf<AFlyingPlatform> FlyingPlatformClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Classes")
    TSubclassOf<AFlyingAttacker> FlyingAttackerClass;

    // =====================================================
    // HallucinationGhost (Insanity 80+ 소환)
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HallucinationGhost")
    TSubclassOf<AHallucinationGhost> HallucinationGhostClass;

    // 소환할 고스트 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HallucinationGhost",
        meta = (ClampMin = "0"))
    int32 HallucinationGhostCount = 3;

    // 고스트 간 최소 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HallucinationGhost",
        meta = (ClampMin = "0.0"))
    float HallucinationGhostMinDistance = 800.0f;

    // StageHeightMax — CliffTotalHeight 기반 자동 설정 (0이면 자동)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HallucinationGhost",
        meta = (ClampMin = "0.0"))
    float HallucinationGhostStageHeightMax = 0.0f;

    // =====================================================
    // WallCrawler Rules
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WallMinAbsNormalZ = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WallMaxAbsNormalZ = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler", meta = (ClampMin = "0.0"))
    float WallCrawlerSpawnOffset = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler", meta = (ClampMin = "0.0"))
    float WallCrawlerFacingProbeDepth = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler")
    FRotator WallCrawlerVisualRotationOffset = FRotator::ZeroRotator;

    // =====================================================
    // FlyingPlatform Rules
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformMinWallDistance = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformMaxWallDistance = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformMinHeightAboveGround = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformMaxHeightAboveGround = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformClearanceRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformClearanceHalfHeight = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PlatformSupportMaxAbsNormalZ = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformOverlapCheckRadius = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "0.0"))
    float PlatformPushOutStep = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingPlatform", meta = (ClampMin = "1"))
    int32 PlatformMaxPushOutSteps = 6;

    // =====================================================
    // FlyingAttacker Rules
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerMinWallDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerMaxWallDistance = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerMinHeightAboveGround = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerMaxHeightAboveGround = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerLocalClearanceRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerLocalClearanceHalfHeight = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0"))
    int32 AttackerRequiredOpenOctants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AttackerRequiredOctantDistanceRatio = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AttackerRequiredAverageDistanceRatio = 0.60f;

    // =====================================================
    // Debug
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugPoints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bVerboseLog = true;

    // =====================================================
    // API
    // =====================================================
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnMonsters();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void RespawnMonsters();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SetPlacementSeedAndRespawn(int32 NewPlacementSeed);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearSpawnedMonsters();

    // Insanity 80+ 조건 시 DownfallCharacter에서 호출
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnHallucinationGhosts();

    // 현재 스폰된 고스트 전체 제거
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearHallucinationGhosts();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearAllMonsters();

    UFUNCTION(BlueprintCallable, Category = "Spawning|Debug")
    void GetCurrentMonsterCounts(
        int32& OutWallCrawlerCount,
        int32& OutFlyingPlatformCount,
        int32& OutFlyingAttackerCount,
        int32& OutTotalCount) const;

    UFUNCTION(BlueprintCallable, Category = "Spawning|Debug")
    FString GetCurrentMonsterCountDebugText() const;

    UFUNCTION()
    void HandleMountainGenerated(AActor* Generator);

private:
    bool ResolveMountain();
    bool GetMountainBounds(FBox& OutBounds) const;
    bool GetFrontBandBounds(FBox& OutBounds) const;

    float GetMinDistanceForKind(EMonsterSpawnKind Kind) const;
    bool IsFarEnoughFromSameType(const FVector& Location, EMonsterSpawnKind Kind) const;
    float GetNearestSameTypeDistance(const FVector& Location, EMonsterSpawnKind Kind) const;

    bool IsInsideRock(const FVector& Location, float CheckDistance = 60.0f) const;
    bool HasLocalClearance(const FVector& Location, float Radius, float HalfHeight) const;
    bool TraceHeightAboveGround(const FVector& Location, float& OutHeight) const;
    bool IsSphereOverlappingWorldStatic(const FVector& Location, float Radius) const;
    bool PushOutFromWall(FVector& InOutLocation, const FVector& WallNormal, float Radius) const;

    bool TraceMountainOnly(const FVector& Start, const FVector& End, FHitResult& OutHit) const;
    bool SampleMountainSurface(FRandomStream& Stream, FHitResult& OutHit, float MinAbsNormalZ, float MaxAbsNormalZ, ESpawnFailReason* OutFailReason = nullptr) const;
    bool SampleFrontBandAirLocation(FRandomStream& Stream, FVector& OutLocation, float HorizontalRadius, float MinZOffset, float MaxZOffset, EMonsterSpawnKind Kind) const;
    bool IsPointWithinFrontSpawnBand(const FVector& WorldPoint) const;
    bool IsFrontFacingSurface(const FHitResult& Hit) const;
    bool TraceFrontCliffFromAir(const FVector& AirLocation, float TraceDistance, FHitResult& OutHit) const;

    bool FindWallCrawlerSpawn(FRandomStream& Stream, FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const;
    bool FindFlyingPlatformSpawn(FRandomStream& Stream, FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const;
    bool FindFlyingAttackerSpawn(FRandomStream& Stream, FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const;

    bool PrepareSpawnResults(
        TArray<FSpawnProbeResult>& OutWallResults,
        TArray<FSpawnProbeResult>& OutPlatformResults,
        TArray<FSpawnProbeResult>& OutAttackerResults,
        FSpawnFailStats& OutWallStats,
        FSpawnFailStats& OutPlatformStats,
        FSpawnFailStats& OutAttackerStats) const;

    bool SpawnPreparedMonsters(
        const TArray<FSpawnProbeResult>& WallResults,
        const TArray<FSpawnProbeResult>& PlatformResults,
        const TArray<FSpawnProbeResult>& AttackerResults,
        TArray<AMonsterBase*>& OutSpawnedMonsters,
        FSpawnFailStats& InOutWallStats,
        FSpawnFailStats& InOutPlatformStats,
        FSpawnFailStats& InOutAttackerStats) const;

    bool EvaluateFlyingAttackerTerritory(const FVector& CandidateLocation, float TerritoryRadius) const;
    int32 MakePlacementStreamSeed(EMonsterSpawnKind Kind) const;

    static const TCHAR* GetFailReasonText(ESpawnFailReason Reason);
    void LogFailStats(const TCHAR* Label, const FSpawnFailStats& Stats, int32 Spawned, int32 Target) const;
    void TryInitialSpawnFallback();

private:
    UPROPERTY()
    TArray<AMonsterBase*> SpawnedMonsters;

    UPROPERTY()
    TArray<AActor*> SpawnedGhosts;

    UPROPERTY(Transient)
    int32 LastObservedPlacementSeed = 12345;

    FTimerHandle DeferredInitialSpawnTimer;
};