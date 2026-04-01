#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MountainGenSettings.h"
#include "MountainGenMeshData.h"
#include "MountainGenWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

struct FMGMetrics;

USTRUCT()
struct FMGAsyncResult
{
    GENERATED_BODY()

    bool bValid = false;
    int32 BuildSerial = 0;

    FMountainGenSettings FinalSettings;
    FChunkMeshData MeshData;
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
        const FVector TerrainOrigin = GetTerrainOriginWorld();
        const float FrontX = FMath::Max(200.f, Settings.CliffThicknessCm);
        const float LocalX = WorldPoint.X - TerrainOrigin.X;
        return (FrontX - LocalX);
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization")
    bool bEnablePostWeld = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization", meta = (ClampMin = "0.01"))
    float PostWeldEpsilonScale = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization")
    bool bEnableIslandCull = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Optimization", meta = (ClampMin = "1"))
    int32 MinTrisToKeepAfterCull = 200;

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