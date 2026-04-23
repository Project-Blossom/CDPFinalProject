#pragma once

#include "CoreMinimal.h"
#include "Monsters/FlyingMonster.h"
#include "Climbing/IClimbableSurface.h"
#include "FlyingPlatform.generated.h"

UCLASS()
class PROTOTYPE_API AFlyingPlatform : public AFlyingMonster, public IClimbableSurface
{
    GENERATED_BODY()

public:
    AFlyingPlatform();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ============================================
    // IClimbableSurface 구현
    // ============================================
    virtual bool FindNearestGripPoint_Implementation(const FVector& SearchOrigin, float SearchRadius, FGripPointInfo& OutGripInfo) override;

    // ============================================
    // Platform State
    // ============================================
    UPROPERTY(BlueprintReadOnly, Category = "Platform")
    bool bPlayerAttached = false;

    UPROPERTY(BlueprintReadOnly, Category = "Platform")
    float AttachedTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Platform")
    float DescendStartTime = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Platform")
    float DescendSpeed = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Platform")
    FName GrabSocketName = "GrabSocket";
    
    UPROPERTY(EditAnywhere, Category = "Platform")
    float GripDetectionRadius = 300.0f; // FlyingPlatform 전용 잡기 감지 반경 (cm)
    
    // ============================================
    // Carrier System (WallCrawler Transport)
    // ============================================
    UPROPERTY(EditAnywhere, Category = "Carrier")
    float WallCrawlerDetectionRadius = 5000.0f;  // 50m
    
    UPROPERTY(EditAnywhere, Category = "Carrier")
    float PlayerDropRadius = 5000.0f;  // 50m
    
    UPROPERTY(EditAnywhere, Category = "Carrier")
    float CrawlerSearchInterval = 5.0f;  // 5초
    
    UPROPERTY(BlueprintReadOnly, Category = "Carrier")
    class AWallCrawler* CarriedCrawler = nullptr;
    
    UPROPERTY(BlueprintReadOnly, Category = "Carrier")
    float LastSearchTime = 0.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Carrier")
    class AWallCrawler* FindNearbyWallCrawler();
    
    UFUNCTION(BlueprintCallable, Category = "Carrier")
    void DropWallCrawler();
    
    UFUNCTION(BlueprintPure, Category = "Carrier")
    bool HasCarriedCrawler() const { return CarriedCrawler != nullptr; }
    
    UFUNCTION(BlueprintPure, Category = "Carrier")
    bool CanDropCrawler() const;

#if !UE_BUILD_SHIPPING
    // CanDropCrawler 디버그 시각화 최초 1회 플래그
    // const 함수 내에서 수정 가능하도록 mutable 선언
    mutable bool bHasDrawnDropDebug = false;
#endif
    
    // ============================================
    // Patrol State
    // ============================================
    UPROPERTY(BlueprintReadOnly, Category = "Flight")
    FVector CurrentTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Flight")
    bool bHasTarget = false;

    UPROPERTY(EditAnywhere, Category = "Flight")
    float ArrivalThreshold = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Flight")
    float IdleWaitTime = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Flight")
    float IdleTimer = 0.0f;

    // ============================================
    // Functions
    // ============================================
    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OnPlayerGrab(class ADownfallCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OnPlayerRelease();

    UFUNCTION(BlueprintPure, Category = "Platform")
    FVector GetGrabLocation() const;

    UFUNCTION()
    void UpdatePatrol(float DeltaTime);

    virtual void Attack() override {}

protected:
    // Override: 장애물 감지 시 목표 재설정
    virtual void OnObstacleDetected(const FVector& ObstacleDirection) override;
};