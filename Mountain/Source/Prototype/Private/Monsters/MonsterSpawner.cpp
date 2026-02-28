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
    
    if (bAutoSpawnOnBeginPlay)
    {
        SpawnMonsters();
    }
}

void AMonsterSpawner::SpawnMonsters()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterSpawner: No valid world!"));
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterSpawner: No player found!"));
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector SpawnerLocation = GetActorLocation();
    
    ClearAllMonsters();
    
    // 몬스터 타입별 개수 계산 (1:1:1)
    int32 CountPerType = TotalMonsterCount / 3;
    int32 Remainder = TotalMonsterCount % 3;
    
    int32 WallCrawlerCount = CountPerType + (Remainder > 0 ? 1 : 0);
    int32 FlyingPlatformCount = CountPerType + (Remainder > 1 ? 1 : 0);
    int32 FlyingAttackerCount = CountPerType;
    
    UE_LOG(LogTemp, Log, TEXT("MonsterSpawner: Spawning %d WallCrawlers, %d FlyingPlatforms, %d FlyingAttackers"),
        WallCrawlerCount, FlyingPlatformCount, FlyingAttackerCount);
    
    int32 MaxAttempts = TotalMonsterCount * 20;
    
    // WallCrawler 스폰
    int32 SuccessCount = 0;
    for (int32 Attempt = 0; Attempt < MaxAttempts && SuccessCount < WallCrawlerCount; Attempt++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f)
        );
        FVector SpawnLocation = SpawnerLocation + RandomOffset;
        
        if (!IsValidSpawnLocation(SpawnLocation, true, PlayerLocation))
            continue;
        
        FVector WallNormal;
        GetDistanceToNearestWall(SpawnLocation, WallNormal);
        FRotator SpawnRotation = (-WallNormal).Rotation();
        
        if (WallCrawlerClass)
        {
            AWallCrawler* Monster = GetWorld()->SpawnActor<AWallCrawler>(
                WallCrawlerClass, SpawnLocation, SpawnRotation);
            
            if (Monster)
            {
                SpawnedMonsters.Add(Monster);
                SuccessCount++;
                
                if (bShowDebugSpheres)
                {
                    DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 12, FColor::Green, true, 10.0f, 0, 3.0f);
                }
                
                UE_LOG(LogTemp, Log, TEXT("WallCrawler spawned at distance %.1fcm"), 
                    FVector::Dist(SpawnLocation, PlayerLocation));
            }
        }
    }
    
    // FlyingPlatform 스폰
    SuccessCount = 0;
    for (int32 Attempt = 0; Attempt < MaxAttempts && SuccessCount < FlyingPlatformCount; Attempt++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f)
        );
        FVector SpawnLocation = SpawnerLocation + RandomOffset;
        
        if (!IsValidSpawnLocation(SpawnLocation, false, PlayerLocation))
            continue;
        
        if (FlyingPlatformClass)
        {
            AFlyingPlatform* Monster = GetWorld()->SpawnActor<AFlyingPlatform>(
                FlyingPlatformClass, SpawnLocation, FRotator::ZeroRotator);
            
            if (Monster)
            {
                SpawnedMonsters.Add(Monster);
                SuccessCount++;
                
                if (bShowDebugSpheres)
                {
                    DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 12, FColor::Blue, true, 10.0f, 0, 3.0f);
                }
            }
        }
    }
    
    // FlyingAttacker 스폰
    SuccessCount = 0;
    for (int32 Attempt = 0; Attempt < MaxAttempts && SuccessCount < FlyingAttackerCount; Attempt++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f)
        );
        FVector SpawnLocation = SpawnerLocation + RandomOffset;
        
        if (!IsValidSpawnLocation(SpawnLocation, false, PlayerLocation))
            continue;
        
        if (FlyingAttackerClass)
        {
            AFlyingAttacker* Monster = GetWorld()->SpawnActor<AFlyingAttacker>(
                FlyingAttackerClass, SpawnLocation, FRotator::ZeroRotator);
            
            if (Monster)
            {
                SpawnedMonsters.Add(Monster);
                SuccessCount++;
                
                if (bShowDebugSpheres)
                {
                    DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 12, FColor::Red, true, 10.0f, 0, 3.0f);
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

bool AMonsterSpawner::IsValidSpawnLocation(const FVector& Location, bool bIsWallCrawler, const FVector& PlayerLocation)
{
    // 1. 플레이어와 거리 체크
    float DistanceToPlayer = FVector::Dist(Location, PlayerLocation);
    if (DistanceToPlayer < MinDistanceFromPlayer)
        return false;
    
    // 2. 암벽 안 체크
    if (IsInsideRock(Location))
        return false;
    
    // 3. 벽 거리 체크
    FVector WallNormal;
    float DistanceToWall = GetDistanceToNearestWall(Location, WallNormal);
    
    if (bIsWallCrawler)
    {
        // WallCrawler: 벽 근처여야 함
        if (DistanceToWall < WallCrawlerMinWallDistance || DistanceToWall > WallCrawlerMaxWallDistance)
            return false;
    }
    else
    {
        // Flying: 벽이 너무 멀면 안 됨
        if (DistanceToWall > FlyingMaxWallDistance)
            return false;
    }
    
    return true;
}

float AMonsterSpawner::GetDistanceToNearestWall(const FVector& Location, FVector& OutWallNormal)
{
    TArray<FVector> Directions = {
        FVector::ForwardVector, -FVector::ForwardVector,
        FVector::RightVector, -FVector::RightVector,
        FVector::UpVector, -FVector::UpVector
    };
    
    float MinDistance = 10000.0f;
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
    
    // 위아래 둘 다 가까이 막혀 있으면 암벽 안
    if (bHitUp && bHitDown && HitUp.Distance < 50.0f && HitDown.Distance < 50.0f)
        return true;
    
    return false;
}