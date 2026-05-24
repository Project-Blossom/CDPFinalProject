#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MountainGenSettings.h"
#include "MountainGenMeshData.h"
#include "GDPCGTypes.h"
#include "MountainGenWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

struct FMGMetrics;

// ============================================================
// Goal-driven terrain data shared with placement/gameplay modules
// ============================================================

UENUM(BlueprintType)
enum class EMGSurfaceType : uint8
{
    Unknown   UMETA(DisplayName = "Unknown"),
    Ground    UMETA(DisplayName = "Ground"),
    Wall      UMETA(DisplayName = "Wall"),
    Cliff     UMETA(DisplayName = "Cliff"),
    Overhang  UMETA(DisplayName = "Overhang"),
    Steep     UMETA(DisplayName = "Steep"),
    Blocked   UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EMGPlacementUsage : uint8
{
    Monster  UMETA(DisplayName = "Monster"),
    Item     UMETA(DisplayName = "Item"),
    Platform UMETA(DisplayName = "Platform"),
    Gameplay UMETA(DisplayName = "Gameplay")
};

USTRUCT(BlueprintType)
struct FMGSurfaceUsageFlags
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bVisual = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bCollision = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bGameplay = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bPlacement = false;
};

USTRUCT(BlueprintType)
struct FMGSurfaceSample
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    FVector Normal = FVector::UpVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    float SlopeAngleDeg = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    EMGSurfaceType SurfaceType = EMGSurfaceType::Unknown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    FMGSurfaceUsageFlags Usage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bCanPlaceMonster = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bCanPlaceItem = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    bool bCanPlacePlatform = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Surface")
    float DangerScore = 0.f;
};

USTRUCT(BlueprintType)
struct FMGGenerationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    FName ZoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    float RelativeZMinCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    float RelativeZMaxCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    float TargetOverhangMin = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    float TargetOverhangMax = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    float TargetSteepMin = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zone")
    float TargetSteepMax = 1.f;
};

USTRUCT(BlueprintType)
struct FMGZoneMetricReport
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    FName ZoneName = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    int32 SampleCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float OverhangRatio = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float SteepRatio = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float PlacementRatio = 0.f;
};

USTRUCT(BlueprintType)
struct FMGMeshQualityReport
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    int32 VertexCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    int32 TriangleCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    int32 RemovedBadTriangleCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float BadTriangleRatio = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float ThinTriangleRatio = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float NormalRiskRatio = 0.f;
};

USTRUCT(BlueprintType)
struct FMGGenerationReport
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    int32 FinalSeed = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    float FinalScore = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    bool bPassedAllTargets = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    TArray<FGDPCGMetricValue> Metrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    FMGMeshQualityReport MeshQuality;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    TArray<FMGZoneMetricReport> ZoneReports;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    int32 SurfaceSampleCount = 0;
};


USTRUCT()
struct FMGAsyncResult
{
    GENERATED_BODY()

    bool bValid = false;
    int32 BuildSerial = 0;

    FMountainGenSettings FinalSettings;
    FChunkMeshData MeshData;

    int32 RemovedBadTriangleCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMountainGenerated, AMountainGenWorldActor*, Generator);

UCLASS()
class MOUNTAINGEN_API AMountainGenWorldActor : public AActor
{
    GENERATED_BODY()

public:
    AMountainGenWorldActor();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // ---------- API ----------
    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void Regenerate();

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void SetSeed(int32 NewSeed);

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void RandomizeSeed();

    UFUNCTION()
    void CycleDifficulty();

    UFUNCTION(BlueprintCallable, Category = "MountainGen|Debug")
    void ToggleOnScreenMessages();

    UFUNCTION(BlueprintCallable, Category = "MountainGen|Debug")
    void SetOnScreenMessagesEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "MountainGen|Runtime")
    bool HasGeneratedMesh() const
    {
        return bHasGeneratedMesh;
    }

    UFUNCTION(BlueprintPure, Category = "MountainGen|Runtime")
    FBox GetGeneratedWorldBounds() const
    {
        return GeneratedWorldBounds;
    }

    UFUNCTION(BlueprintPure, Category = "MountainGen|Spawn")
    FVector GetTerrainOriginWorld() const
    {
        const float FrontX = FMath::Max(200.f, Settings.CliffThicknessCm);
        return GetActorLocation() - FVector(FrontX, 0.f, 0.f);
    }

    UFUNCTION(BlueprintPure, Category = "MountainGen|Spawn")
    FVector GetFrontDirectionWorld() const
    {
        return FVector::ForwardVector;
    }

    UFUNCTION(BlueprintPure, Category = "MountainGen|Spawn")
    float GetFrontSurfaceWorldX() const
    {
        return GetActorLocation().X;
    }

    UFUNCTION(BlueprintPure, Category = "MountainGen|Spawn")
    float GetSuggestedFrontSpawnDepthCm() const
    {
        if (Settings.FrontBandDepthCm > 0.f)
        {
            return Settings.FrontBandDepthCm;
        }

        return FMath::Clamp(Settings.CliffDepthCm * 0.25f, 1200.f, 6000.f);
    }

    UFUNCTION(BlueprintPure, Category = "MountainGen|Spawn")
    float GetSignedFrontDepthCm(const FVector& WorldPoint) const
    {
        // Actual cliff front air is on +X side.
        // Terrain density uses (FrontX - LocalX), so solid is LocalX < FrontX
        // and the visible/front air side is LocalX > FrontX.
        return WorldPoint.X - GetFrontSurfaceWorldX();
    }

    UFUNCTION(BlueprintCallable, Category = "MountainGen|Query")
    bool QuerySurfaceAtLocation(const FVector& WorldLocation, FMGSurfaceSample& OutSample, float SearchRadiusCm = 300.f) const;

    UFUNCTION(BlueprintCallable, Category = "MountainGen|Query")
    void GetPlacementCandidates(TArray<FMGSurfaceSample>& OutCandidates, EMGPlacementUsage Usage) const;

    UFUNCTION(BlueprintCallable, Category = "MountainGen|Query")
    bool IsLocationValidForPlacement(const FVector& WorldLocation, EMGPlacementUsage Usage, float SearchRadiusCm = 300.f) const;

    UFUNCTION(BlueprintCallable, Category = "MountainGen|Query")
    void GetGeneratedSurfaceSamples(TArray<FMGSurfaceSample>& OutSamples) const;

    UFUNCTION(BlueprintPure, Category = "MountainGen|Report")
    FMGGenerationReport GetLastGenerationReport() const
    {
        return LastGenerationReport;
    }

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen")
    TObjectPtr<UProceduralMeshComponent> ProcMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Mesh")
    TObjectPtr<UMaterialInterface> VoxelMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen")
    FMountainGenSettings Settings;

    // ---------- Runtime ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Runtime")
    bool bEnableRandomSeedKey = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Runtime")
    bool bEnableOnScreenToggleKey = true;

    UPROPERTY(BlueprintAssignable, Category = "MountainGen|Runtime")
    FOnMountainGenerated OnMountainGenerated;

    // ---------- Debug ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug")
    bool bEnableOnScreenMessages = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug")
    bool bDebugSeedSearch = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug")
    bool bDebugPipeline = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug", meta = (ClampMin = "1", ClampMax = "200"))
    int32 DebugPrintEveryNAttempt = 10;

    // ---------- Optimization ----------
    // 선택 사항: Marching Cubes 직후 1차 Weld.
    // 형상 자체를 많이 바꾸고 싶지 않으면 false로 두고, 아래 Seam Repair만 사용한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization")
    bool bEnablePostWeld = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization", meta = (ClampMin = "0.001"))
    float PostWeldEpsilonScale = 0.15f;

    // 최종 출력 직전의 균열/슬랩 경계/중복 정점 보정용 Weld.
    // 액터에 저장된 bEnablePostWeld 값이 false여도 이 값이 true면 마지막에 한 번 더 안전하게 정점을 통합한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization")
    bool bRepairMeshSeams = true;

    // VoxelSizeCm에 곱해지는 최종 Seam Weld 거리.
    // 0.02면 VoxelSize 100cm 기준 2cm로, 눈에 보이는 균열만 닫고 형상 손상은 줄인다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization", meta = (ClampMin = "0.001", ClampMax = "0.25"))
    float MeshSeamWeldEpsilonScale = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization")
    bool bEnableIslandCull = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization", meta = (ClampMin = "1"))
    int32 MinTrisToKeepAfterCull = 200;

    // ---------- Goal-driven Surface Data / Query ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Surface", meta = (ClampMin = "1", ClampMax = "256"))
    int32 SurfaceSampleTriangleStride = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Surface", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float MonsterMaxSlopeDeg = 42.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Surface", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ItemMaxSlopeDeg = 55.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Surface", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float PlatformMaxSlopeDeg = 75.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Zones")
    TArray<FMGGenerationZone> GenerationZones;

    UPROPERTY(Transient)
    TArray<FMGSurfaceSample> GeneratedSurfaceSamples;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen|Report")
    FMGGenerationReport LastGenerationReport;

    // ---------- Material : Snow / Rock Auto Blend ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material|SnowRock")
    float SnowSlopeMinZ = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material|SnowRock")
    float SnowSlopeMaxZ = 0.8660254f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material|SnowRock")
    float OverhangMaxZ = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material|SnowRock", meta = (ClampMin = "0.0"))
    float SnowNoiseScale = 0.0008f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Material|SnowRock", meta = (ClampMin = "0.0"))
    float SnowNoiseStrength = 0.20f;

private:
    void BuildChunkAndMesh();
    void ApplyGeneratedMeshResult(FMGAsyncResult&& Result, bool bShowRuntimeSeedMessage);
    void UI_Status(const FString& Msg, float Seconds = 2.0f, FColor Color = FColor::Cyan) const;
    void ApplyVoxelMaterialParameters();
    void UpdateGeneratedMeshStateAndBroadcast();

    static FString MakeMetricsLine(
        const FMountainGenSettings& S,
        const FMGMetrics& M,
        bool& bOutOverhangOK,
        bool& bOutSteepOK);

#if WITH_EDITOR
    uint32 ComputeSettingsHash_Editor() const;
#endif

private:
    bool bAsyncWorking = false;
    bool bRegenQueued = false;
    int32 CurrentBuildSerial = 0;
    int32 InFlightBuildSerial = 0;

    TArray<FLinearColor> ReusableColors;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> VoxelMID = nullptr;

    UPROPERTY(Transient)
    bool bHasGeneratedMesh = false;

    UPROPERTY(Transient)
    FBox GeneratedWorldBounds = FBox(EForceInit::ForceInit);

#if WITH_EDITOR
    FTimerHandle EditorRegenTimer;
    float EditorRegenDebounceSeconds = 0.12f;

    void RequestEditorRegen();
    void DoEditorRegen();
#endif

#if WITH_EDITOR
    uint32 LastSettingsHash_Editor = 0;
#endif
};