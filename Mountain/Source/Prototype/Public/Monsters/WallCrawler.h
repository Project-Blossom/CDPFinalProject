#pragma once

#include "CoreMinimal.h"
#include "Monsters/MonsterBase.h"
#include "WallCrawler.generated.h"

UCLASS()
class PROTOTYPE_API AWallCrawler : public AMonsterBase
{
    GENERATED_BODY()

public:
    AWallCrawler();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ============================================
    // Wall Movement
    // ============================================
    UPROPERTY(EditAnywhere, Category = "Movement")
    float CrawlSpeed = 200.0f;              // 기어가는 속도

    UPROPERTY(EditAnywhere, Category = "Movement")
    float CircleRadius = 300.0f;            // 원형 배회 반경

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WallTraceDistance = 100.0f;       // 벽 감지 거리

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WallStickDistance = 20.0f;        // 벽에서 떨어진 거리

    // ============================================
    // Wall State
    // ============================================
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector CurrentWallNormal;              // 현재 벽 Normal

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsOnWall = false;                 // 벽에 붙어있는지

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector WallHitLocation;                // 벽 접촉 위치

    // ============================================
    // Patrol State
    // ============================================
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector PatrolCenter;                   // 배회 중심점

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PatrolAngle = 0.0f;               // 원형 배회 각도

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PatrolAngularSpeed = 30.0f;       // 각속도 (도/초)

    // ============================================
    // Functions
    // ============================================
    
    // Wall Detection
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool DetectWall(FVector& OutWallNormal, FVector& OutHitLocation);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateWallAlignment();             // 벽에 맞춰 회전

    // Movement
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void CrawlOnWall(FVector Direction, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void CirclePatrol(float DeltaTime);     // 원형 배회

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector ProjectToWallSurface(FVector WorldDirection);  // 벽 표면에 방향 투영

    // Override
    virtual void Attack() override {}       // Phase 2에서 구현
};