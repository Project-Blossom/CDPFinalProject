#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

class AMonsterBase;
class AWallCrawler;
class AFlyingPlatform;
class AFlyingAttacker;

UCLASS()
class PROTOTYPE_API AMonsterSpawner : public AActor
{
    GENERATED_BODY()
    
public:    
    AMonsterSpawner();
    virtual void BeginPlay() override;

public:    
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    int32 TotalMonsterCount = 10;
    
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float SpawnRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float MinDistanceFromPlayer = 2000.0f;
    
    // Monster Types
    UPROPERTY(EditAnywhere, Category = "Monster Types")
    TSubclassOf<AWallCrawler> WallCrawlerClass;
    
    UPROPERTY(EditAnywhere, Category = "Monster Types")
    TSubclassOf<AFlyingPlatform> FlyingPlatformClass;
    
    UPROPERTY(EditAnywhere, Category = "Monster Types")
    TSubclassOf<AFlyingAttacker> FlyingAttackerClass;
    
    // WallCrawler Settings
    UPROPERTY(EditAnywhere, Category = "WallCrawler Settings")
    float WallCrawlerMinWallDistance = 100.0f;
    
    UPROPERTY(EditAnywhere, Category = "WallCrawler Settings")
    float WallCrawlerMaxWallDistance = 300.0f;
    
    // Flying Settings
    UPROPERTY(EditAnywhere, Category = "Flying Settings")
    float FlyingMinWallDistance = 300.0f;    // Flying은 벽에서 최소 3m
    
    UPROPERTY(EditAnywhere, Category = "Flying Settings")
    float FlyingMaxWallDistance = 1500.0f;   // Flying은 벽에서 최대 15m
    
    UPROPERTY(EditAnywhere, Category = "Flying Settings")
    float FlyingMinHeight = 200.0f;          // 최소 높이 2m
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebugSpheres = false;
    
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bAutoSpawnOnBeginPlay = true;

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnMonsters();
    
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearAllMonsters();
    
private:
    bool IsValidSpawnLocation(const FVector& Location, bool bIsWallCrawler, const FVector& PlayerLocation);
    float GetDistanceToNearestWall(const FVector& Location, FVector& OutWallNormal);
    bool IsInsideRock(const FVector& Location);
    
    UPROPERTY()
    TArray<AMonsterBase*> SpawnedMonsters;
};