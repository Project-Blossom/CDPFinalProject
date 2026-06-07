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
struct FItemSpawnEntry
{
    GENERATED_BODY()

    // ItemDefinition에 등록된 ItemId와 동일해야 한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemId = NAME_None;

    // 이 아이템 드롭 액터를 몇 개 생성할지.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
    int32 SpawnCount = 1;

    // 드롭 액터 하나가 들고 있는 아이템 개수.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 CountPerDrop = 1;

    // 월드 표시용 메시.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
    TObjectPtr<UStaticMesh> VisualMesh = nullptr;

    // 특정 아이템만 다른 드롭 액터 BP를 쓰고 싶을 때 설정한다.
    // 비워두면 ItemSpawner의 DefaultDropActorClass를 사용한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Class")
    TSubclassOf<AItemDropActor> DropActorClass = nullptr;

    // 0이면 ItemSpawner가 메시 크기와 기본 여백을 이용해 자동 계산한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Placement", meta = (ClampMin = "0.0"))
    float SurfaceOffsetOverrideCm = 0.0f;
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

    // 목표 기반 절차적 생성 플러그인에서 산/절벽 생성 완료 후 아이템을 배치한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bSpawnOnlyAfterMountainGenerated = true;

    // 산이 재생성되면 기존 드롭을 지우고 다시 배치할지.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain")
    bool bRespawnWhenMountainRegenerates = true;

    // 루트용 컴포넌트. 스폰 범위로 사용하지 않는다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    TObjectPtr<USceneComponent> SceneRoot = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bClearExistingDropsBeforeSpawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AItemDropActor> DefaultDropActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
    TArray<FItemSpawnEntry> ItemsToSpawn;

    // =====================================================
    // Deterministic Cliff Front Placement
    // =====================================================

    // 절벽 전체 Y 폭에서 좌우 끝을 제외하는 거리.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float SideMarginCm = 300.0f;

    // 절벽 높이 중 아이템을 배치할 구간. 0~1 비율.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnHeightRatioMin = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement|Height", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnHeightRatioMax = 0.92f;

    // 절벽 표면에서 앞면 노말 방향으로 기본으로 띄우는 거리.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float DefaultSurfaceOffsetCm = 20.0f;

    // VisualMesh Bounds를 이용해 아이템 전체가 절벽 밖으로 나오게 할지.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement")
    bool bUseVisualMeshBoundsForOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0"))
    float MeshClearancePaddingCm = 15.0f;

    // +X가 절벽 앞면 방향이다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cliff Front Placement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FrontFacingNormalDotMin = 0.25f;

    // 완성된 절벽 표면 위치를 읽기 위한 Trace 거리.
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
    bool BuildDeterministicSpawnTransform(int32 LinearIndex, int32 TotalCount, const FItemSpawnEntry& Entry, FTransform& OutTransform) const;
    float CalculateSurfaceOffset(const FItemSpawnEntry& Entry) const;
    int32 GetTotalRequestedDropCount() const;
    void TryInitialSpawnFallback();

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<AItemDropActor>> SpawnedDrops;

    FTimerHandle DeferredInitialSpawnTimer;
};