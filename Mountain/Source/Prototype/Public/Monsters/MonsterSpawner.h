#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

class AMonsterBase;
class AWallCrawler;
class AFlyingPlatform;
class AFlyingAttacker;
class AMountainGenWorldActor;

UCLASS()
class PROTOTYPE_API AMonsterSpawner : public AActor
{
    GENERATED_BODY()
    
public:    
    AMonsterSpawner();

protected:
    virtual void BeginPlay() override;

public:    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 TotalMonsterCount = 10;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float SpawnRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float MinDistanceFromPlayer = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Types")
    TSubclassOf<AWallCrawler> WallCrawlerClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Types")
    TSubclassOf<AFlyingPlatform> FlyingPlatformClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Types")
    TSubclassOf<AFlyingAttacker> FlyingAttackerClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler Settings")
    float WallCrawlerMinWallDistance = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallCrawler Settings")
    float WallCrawlerMaxWallDistance = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying Settings")
    float FlyingMaxWallDistance = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo = false;
    
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnMonsters();
    
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearAllMonsters();

private:
    bool IsValidSpawnLocation(const FVector& Location, bool bIsWallCrawler);
    float GetDistanceToNearestWall(const FVector& Location, FVector& OutWallNormal);
    bool IsInsideRock(const FVector& Location);
    
    UPROPERTY()
    TArray<AMonsterBase*> SpawnedMonsters;
};