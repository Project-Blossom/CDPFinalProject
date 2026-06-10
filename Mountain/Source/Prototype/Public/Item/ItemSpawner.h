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

    // 0이면 ItemSpawner의 SurfaceDistanceFromCliffCm 값을 사용한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Placement", meta = (ClampMin = "0.0"))
    float SurfaceDistanceOverrideCm = 0.0f;
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

    // =====================================================
    // Goal Based Cliff Front Placement
    // =====================================================

    // 절벽 전체 Y 폭에서 좌우 끝을 제외하는 거리.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float SideMarginCm = 300.0f;

    // 절벽 높이 중 아이템을 배치할 구간. 0~1 비율.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnHeightRatioMin = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnHeightRatioMax = 0.92f;

    // 절벽 앞면의 실제 노이즈 표면에서 1m 떨어진 위치에 아이템 중심을 둔다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float SurfaceDistanceFromCliffCm = 100.0f;

    // true면 메시 반지름만큼 더 밀어내서 아이템 전체가 절벽 밖으로 나오게 한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement")
    bool bAddVisualMeshRadiusToDistance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float MeshClearancePaddingCm = 0.0f;

    // +X 방향을 바라보는 표면만 절벽 앞면으로 인정한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FrontFacingNormalDotMin = 0.25f;

    // 실제 노이즈 표면을 찾기 위해 +X 앞쪽에서 -X 방향으로 쏘는 Trace 거리.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Trace", meta = (ClampMin = "100.0"))
    float FrontTraceStartDistanceCm = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Trace", meta = (ClampMin = "100.0"))
    float FrontTraceBackDistanceCm = 30000.0f;

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
    bool TraceFrontSurfaceAtYZ(float Y, float Z, FHitResult& OutHit) const;
    bool BuildSpawnTransform(int32 LinearIndex, int32 TotalCount, const FCliffItemSpawnEntry& Entry, FTransform& OutTransform) const;
    float CalculateSurfaceDistance(const FCliffItemSpawnEntry& Entry) const;
    int32 GetTotalRequestedDropCount() const;
    void TryInitialSpawnFallback();

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<AItemDropActor>> SpawnedDrops;

    FTimerHandle DeferredInitialSpawnTimer;
};