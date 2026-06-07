#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ItemSpawner.generated.h"

class USceneComponent;
class UStaticMesh;
class AItemDropActor;
class AMountainGenWorldActor;

USTRUCT(BlueprintType)
struct FCliffItemSpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
    int32 SpawnCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 CountPerDrop = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
    TObjectPtr<UStaticMesh> VisualMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Class")
    TSubclassOf<AItemDropActor> DropActorClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Placement", meta = (ClampMin = "0.0"))
    float FrontOffsetOverrideCm = 0.0f;
};

UCLASS()
class PROTOTYPE_API AItemSpawner : public AActor
{
    GENERATED_BODY()

public:
    AItemSpawner();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    TObjectPtr<AMountainGenWorldActor> TargetMountain = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bAutoFindMountain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bSpawnOnlyAfterMountainGenerated = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bRespawnWhenMountainRegenerates = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    TObjectPtr<USceneComponent> SceneRoot = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bClearExistingDropsBeforeSpawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AItemDropActor> DefaultDropActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
    TArray<FCliffItemSpawnEntry> CliffItemsToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float SideMarginCm = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnHeightRatioMin = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnHeightRatioMax = 0.92f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float DefaultFrontOffsetCm = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement")
    bool bUseVisualMeshBoundsForOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float MeshClearancePaddingCm = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugSpawnPoints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bVerboseLog = true;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnItems();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void ClearSpawnedItems();

    UFUNCTION(BlueprintPure, Category = "Spawner")
    int32 GetSpawnedDropCount() const { return SpawnedDrops.Num(); }

    UFUNCTION()
    void HandleMountainGenerated(AActor* Generator);

private:
    bool ResolveMountain();
    bool GetMountainBounds(FBox& OutBounds) const;
    bool BuildSpawnTransform(int32 LinearIndex, int32 TotalCount, const FCliffItemSpawnEntry& Entry, FTransform& OutTransform) const;
    float CalculateFrontOffset(const FCliffItemSpawnEntry& Entry) const;
    int32 GetTotalRequestedDropCount() const;
    void TryInitialSpawnFallback();

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<AItemDropActor>> SpawnedDrops;

    FTimerHandle DeferredInitialSpawnTimer;
};