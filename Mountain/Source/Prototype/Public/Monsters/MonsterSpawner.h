#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

class AMonsterBase;
class AWallCrawler;
class AFlyingPlatform;
class AFlyingAttacker;
class AMountainGenWorldActor;

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
    MonsterDistance,
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
    // General Rules
    // =====================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0"))
    float MinDistanceBetweenMonsters = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerWallCrawler = 300;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerFlyingPlatform = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerFlyingAttacker = 600;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0"))
    float BoundsPadding = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General", meta = (ClampMin = "0.0"))
    float FrontSpawnDepthOverrideCm = 6000.0f;

    // 앞면이면 되고 꼭 절벽 바로 옆일 필요는 없게
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    bool bRequireCliffProximityForFlying = false;

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

    // 벽 안 생성 방지용
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
    float AttackerMinWallDistance = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlyingAttacker", meta = (ClampMin = "0.0"))
    float AttackerMaxWallDistance = 1200.0f;

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
    void ClearAllMonsters();

    UFUNCTION()
    void HandleMountainGenerated(AMountainGenWorldActor* Generator);

private:
    bool ResolveMountain();
    bool GetMountainBounds(FBox& OutBounds) const;
    bool GetFrontBandBounds(FBox& OutBounds) const;

    bool IsFarEnoughFromOtherMonsters(const FVector& Location) const;
    bool IsInsideRock(const FVector& Location, float CheckDistance = 60.0f) const;
    bool HasLocalClearance(const FVector& Location, float Radius, float HalfHeight) const;
    bool TraceHeightAboveGround(const FVector& Location, float& OutHeight) const;
    bool IsSphereOverlappingWorldStatic(const FVector& Location, float Radius) const;
    bool PushOutFromWall(FVector& InOutLocation, const FVector& WallNormal, float Radius) const;

    bool TraceMountainOnly(const FVector& Start, const FVector& End, FHitResult& OutHit) const;
    bool SampleMountainSurface(FHitResult& OutHit, float MinAbsNormalZ, float MaxAbsNormalZ, ESpawnFailReason* OutFailReason = nullptr) const;
    bool SampleFrontBandAirLocation(FVector& OutLocation, float HorizontalRadius, float MinZOffset, float MaxZOffset) const;
    bool IsPointWithinFrontSpawnBand(const FVector& WorldPoint) const;

    bool FindWallCrawlerSpawn(FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const;
    bool FindFlyingPlatformSpawn(FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const;
    bool FindFlyingAttackerSpawn(FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const;

    bool EvaluateFlyingAttackerTerritory(const FVector& CandidateLocation, float TerritoryRadius) const;

    static const TCHAR* GetFailReasonText(ESpawnFailReason Reason);
    void LogFailStats(const TCHAR* Label, const FSpawnFailStats& Stats, int32 Spawned, int32 Target) const;
    void TryInitialSpawnFallback();

private:
    UPROPERTY()
    TArray<AMonsterBase*> SpawnedMonsters;

    FTimerHandle DeferredInitialSpawnTimer;
};