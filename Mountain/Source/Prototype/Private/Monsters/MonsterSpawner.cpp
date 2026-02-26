#include "Monsters/MonsterSpawner.h"
#include "Monsters/MonsterBase.h"
#include "Monsters/WallCrawler.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/FlyingAttacker.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AMonsterSpawner::AMonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMonsterSpawner::BeginPlay()
{
    Super::BeginPlay();
}

void AMonsterSpawner::SpawnMonsters()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterSpawner: No valid world!"));
        return;
    }
    
    ClearAllMonsters();
    
    int32 CountPerType = TotalMonsterCount / 3;
    int32 Remainder = TotalMonsterCount % 3;
    
    int32 WallCrawlerCount = CountPerType + (Remainder > 0 ? 1 : 0);
    int32 FlyingPlatformCount = CountPerType + (Remainder > 1 ? 1 : 0);
    int32 FlyingAttackerCount = CountPerType;
    
    UE_LOG(LogTemp, Warning, TEXT("MonsterSpawner: Spawning %d WallCrawlers, %d FlyingPlatforms, %d FlyingAttackers"),
        WallCrawlerCount, FlyingPlatformCount, FlyingAttackerCount);
    
    FVector SpawnerLocation = GetActorLocation();
    int32 MaxAttempts = TotalMonsterCount * 10;
    
    // WallCrawler 스폰
    int32 SuccessCount = 0;
    int32 Attempts = 0;
    while (SuccessCount < WallCrawlerCount && Attempts < MaxAttempts)
    {
        Attempts++;
        
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f)
        );
        FVector SpawnLocation = SpawnerLocation + RandomOffset;
        
        if (IsValidSpawnLocation(SpawnLocation, true))
        {
            FVector WallNormal;
            GetDistanceToNearestWall(SpawnLocation, WallNormal);
            FRotator SpawnRotation = (-WallNormal).Rotation();
            
            if (WallCrawlerClass)
            {
                AWallCrawler* Monster = GetWorld()->SpawnActor<AWallCrawler>(
                    WallCrawlerClass,
                    SpawnLocation,
                    SpawnRotation
                );
                
                if (Monster)
                {
                    SpawnedMonsters.Add(Monster);
                    SuccessCount++;
                    
                    if (bShowDebugInfo)
                    {
                        DrawDebugSphere(GetWorld(), SpawnLocation, 30.0f, 8, FColor::Green, false, 5.0f);
                    }
                }
            }
        }
    }
    
    // FlyingPlatform 스폰
    SuccessCount = 0;
    Attempts = 0;
    while (SuccessCount < FlyingPlatformCount && Attempts < MaxAttempts)
    {
        Attempts++;
        
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f)
        );
        FVector SpawnLocation = SpawnerLocation + RandomOffset;
        
        if (IsValidSpawnLocation(SpawnLocation, false))
        {
            if (FlyingPlatformClass)
            {
                AFlyingPlatform* Monster = GetWorld()->SpawnActor<AFlyingPlatform>(
                    FlyingPlatformClass,
                    SpawnLocation,
                    FRotator::ZeroRotator
                );
                
                if (Monster)
                {
                    SpawnedMonsters.Add(Monster);
                    SuccessCount++;
                    
                    if (bShowDebugInfo)
                    {
                        DrawDebugSphere(GetWorld(), SpawnLocation, 30.0f, 8, FColor::Blue, false, 5.0f);
                    }
                }
            }
        }
    }
    
    // FlyingAttacker 스폰
    SuccessCount = 0;
    Attempts = 0;
    while (SuccessCount < FlyingAttackerCount && Attempts < MaxAttempts)
    {
        Attempts++;
        
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f)
        );
        FVector SpawnLocation = SpawnerLocation + RandomOffset;
        
        if (IsValidSpawnLocation(SpawnLocation, false))
        {
            if (FlyingAttackerClass)
            {
                AFlyingAttacker* Monster = GetWorld()->SpawnActor<AFlyingAttacker>(
                    FlyingAttackerClass,
                    SpawnLocation,
                    FRotator::ZeroRotator
                );
                
                if (Monster)
                {
                    SpawnedMonsters.Add(Monster);
                    SuccessCount++;
                    
                    if (bShowDebugInfo)
                    {
                        DrawDebugSphere(GetWorld(), SpawnLocation, 30.0f, 8, FColor::Red, false, 5.0f);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MonsterSpawner: Successfully spawned %d monsters"), SpawnedMonsters.Num());
}

void AMonsterSpawner::ClearAllMonsters()
{
    for (AMonsterBase* Monster : SpawnedMonsters)
    {
        if (Monster && IsValid(Monster))
        {
            Monster->Destroy();
        }
    }
    
    SpawnedMonsters.Empty();
}

bool AMonsterSpawner::IsValidSpawnLocation(const FVector& Location, bool bIsWallCrawler)
{
    if (IsInsideRock(Location))
    {
        return false;
    }
    
    FVector WallNormal;
    float DistanceToWall = GetDistanceToNearestWall(Location, WallNormal);
    
    if (bIsWallCrawler)
    {
        if (DistanceToWall < WallCrawlerMinWallDistance || DistanceToWall > WallCrawlerMaxWallDistance)
        {
            return false;
        }
    }
    else
    {
        if (DistanceToWall > FlyingMaxWallDistance)
        {
            return false;
        }
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
        if (DistanceToPlayer < MinDistanceFromPlayer)
        {
            return false;
        }
    }
    
    return true;
}

float AMonsterSpawner::GetDistanceToNearestWall(const FVector& Location, FVector& OutWallNormal)
{
    TArray<FVector> Directions = {
        FVector::ForwardVector,
        -FVector::ForwardVector,
        FVector::RightVector,
        -FVector::RightVector,
        FVector::UpVector,
        -FVector::UpVector
    };
    
    float MinDistance = 999999.0f;
    FVector BestNormal = FVector::UpVector;
    
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    for (const FVector& Dir : Directions)
    {
        FHitResult Hit;
        FVector TraceEnd = Location + Dir * 1000.0f;
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, Location, TraceEnd, ECC_WorldStatic, Params))
        {
            if (Hit.Distance < MinDistance)
            {
                MinDistance = Hit.Distance;
                BestNormal = Hit.Normal;
            }
        }
    }
    
    OutWallNormal = BestNormal;
    return MinDistance;
}

bool AMonsterSpawner::IsInsideRock(const FVector& Location)
{
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    FHitResult HitUp, HitDown;
    FVector TraceUp = Location + FVector::UpVector * 100.0f;
    FVector TraceDown = Location - FVector::UpVector * 100.0f;
    
    bool bHitUp = GetWorld()->LineTraceSingleByChannel(HitUp, Location, TraceUp, ECC_WorldStatic, Params);
    bool bHitDown = GetWorld()->LineTraceSingleByChannel(HitDown, Location, TraceDown, ECC_WorldStatic, Params);
    
    if (bHitUp && bHitDown && HitUp.Distance < 50.0f && HitDown.Distance < 50.0f)
    {
        return true;
    }
    
    return false;
}