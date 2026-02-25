#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MountainGenSettings.h"
#include "MountainGenMeshData.h"
#include "MountainGenWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

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

UCLASS()
class MOUNTAINGEN_API AMountainGenWorldActor : public AActor
{
    GENERATED_BODY()

public:
    AMountainGenWorldActor();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

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

    // ---------- Debug ----------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug")
    bool bDebugSeedSearch = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug")
    bool bDebugPipeline = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Debug", meta = (ClampMin = "1", ClampMax = "200"))
    int32 DebugPrintEveryNAttempt = 10;

private:
    void BuildChunkAndMesh();
    void UI_Status(const FString& Msg, float Seconds = 2.0f, FColor Color = FColor::Cyan) const;

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
    FMGAsyncResult PendingResult;

#if WITH_EDITOR
    uint32 LastSettingsHash_Editor = 0;
#endif
};