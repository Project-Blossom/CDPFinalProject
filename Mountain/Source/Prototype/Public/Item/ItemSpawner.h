#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawner.generated.h"

class UBoxComponent;
class UStaticMesh;
class AItemDropActor;

USTRUCT(BlueprintType)
struct FItemSpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
    int32 SpawnCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 CountMin = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 CountMax = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
    TObjectPtr<UStaticMesh> VisualMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Class")
    TSubclassOf<AItemDropActor> DropActorClass = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class PROTOTYPE_API AItemSpawner : public AActor
{
    GENERATED_BODY()

public:
    AItemSpawner();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    TObjectPtr<UBoxComponent> SpawnVolume = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bClearExistingDropsBeforeSpawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bUseFixedRandomSeed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (EditCondition = "bUseFixedRandomSeed"))
    int32 RandomSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AItemDropActor> DefaultDropActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
    TArray<FItemSpawnEntry> ItemsToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "0.0"))
    float MinDistanceBetweenDropsCm = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "1"))
    int32 MaxAttemptsPerDrop = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Ground")
    bool bSnapToGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Ground", meta = (ClampMin = "0.0"))
    float GroundTraceUpCm = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Ground", meta = (ClampMin = "0.0"))
    float GroundTraceDownCm = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Ground")
    TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Ground")
    float GroundSpawnOffsetCm = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugSpawnArea = false;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnItems();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void ClearSpawnedItems();

    UFUNCTION(BlueprintPure, Category = "Spawner")
    int32 GetSpawnedDropCount() const { return SpawnedDrops.Num(); }

private:
    bool TryFindSpawnTransform(FRandomStream& Stream, FTransform& OutTransform) const;
    FVector GetRandomPointInSpawnVolume(FRandomStream& Stream) const;
    bool IsFarEnoughFromExistingDrops(const FVector& Location) const;
    int32 RollDropCount(const FItemSpawnEntry& Entry, FRandomStream& Stream) const;

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<AItemDropActor>> SpawnedDrops;
};