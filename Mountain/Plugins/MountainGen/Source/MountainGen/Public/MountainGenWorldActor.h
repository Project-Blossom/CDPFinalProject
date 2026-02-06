#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MountainGenSettings.h"
#include "MountainGenMeshData.h"
#include "MountainGenWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

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

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void Regenerate();

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void SetSeed(int32 NewSeed);

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void RandomizeSeed();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen")
    TObjectPtr<UProceduralMeshComponent> ProcMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Mesh")
    TObjectPtr<UMaterialInterface> VoxelMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen")
    FMountainGenSettings Settings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Runtime")
    bool bEnableRandomSeedKey = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|AutoTune", meta = (ClampMin = "1", ClampMax = "2000"))
    int32 SeedSearchTries = 300;

private:
    void BuildChunkAndMesh();

    static void ApplyDifficultyPresetTo(FMountainGenSettings& S);
    void ApplyDifficultyPreset();

    void UI_Status(const FString& Msg, float Seconds = 2.0f, FColor Color = FColor::Cyan) const;

private:
    // --- async state ---
    bool bAsyncWorking = false;
    bool bRegenQueued = false;

    int32 CurrentBuildSerial = 0;

    int32 InFlightBuildSerial = 0;

    FMGAsyncResult PendingResult;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};