#pragma once

#include "CoreMinimal.h"
#include "Monsters/FlyingMonster.h"
#include "FlyingAttacker.generated.h"

// 공격 상태
UENUM(BlueprintType)
enum class EAttackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),           // 배회 중
    Pursuing    UMETA(DisplayName = "Pursuing"),       // 추격 중
    Charging    UMETA(DisplayName = "Charging"),       // 돌진 준비
    Attacking   UMETA(DisplayName = "Attacking"),      // 돌진 공격 중
    Cooldown    UMETA(DisplayName = "Cooldown")        // 쿨다운 중
};

UCLASS()
class PROTOTYPE_API AFlyingAttacker : public AFlyingMonster
{
    GENERATED_BODY()

public:
    AFlyingAttacker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    
    // Attack State
    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    EAttackState AttackState = EAttackState::Idle;
    
    // Attack Settings
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackSpeed = 600.0f;              // 돌진 속도 (cm/s)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackRange = 300.0f;              // 공격 시작 거리

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackCooldown = 3.0f;             // 공격 쿨다운 (초)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float InsanityDamage = 10.0f;            // 공격 시 Insanity 증가량

    UPROPERTY(EditAnywhere, Category = "Attack")
    float ChargeTime = 0.5f;                 // 돌진 준비 시간 (초)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float PursuitDistance = 1000.0f;         // 추격 시작 거리

    UPROPERTY(EditAnywhere, Category = "Attack")
    float SafeDistance = 500.0f;             // 안전 거리 (공격 후 유지)
    
    // Attack State Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float ChargeStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    FVector AttackTargetLocation;            // 돌진 목표 위치

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    bool bHasHitPlayer = false;              // 이번 공격에서 플레이어 타격 여부
    
    // Functions
    // Attack override
    virtual void Attack() override;

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void StartCharging();                    // 돌진 준비 시작

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void ExecuteCharge();                    // 돌진 실행

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void OnAttackHit(class ADownfallCharacter* Player);  // 공격 적중

    UFUNCTION(BlueprintPure, Category = "Attack")
    bool CanAttack() const;                  // 공격 가능 여부

    UFUNCTION(BlueprintPure, Category = "Attack")
    float GetDistanceToPlayer() const;       // 플레이어와의 거리

    // State Management
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateIdlePatrol(float DeltaTime);  // Idle 배회
    void SetAttackState(EAttackState NewState);
    
    // Patrol State (FlyingPlatform과 동일)
    FVector CurrentPatrolTarget;
    bool bHasPatrolTarget = false;
    float PatrolArrivalThreshold = 100.0f;
    float PatrolIdleTimer = 0.0f;
    float PatrolIdleWaitTime = 2.0f;
    
    // Territory System (영역 기반 배회)
    UPROPERTY(BlueprintReadOnly, Category = "AI|Territory")
    FVector TerritoryCenter;  // 스폰 위치 (영역 중심)
    
    UPROPERTY(EditAnywhere, Category = "AI|Territory")
    float TerritoryRadius = 1000.0f;  // 10m (WallCrawler SightRadius와 동일)
    
    UPROPERTY(EditAnywhere, Category = "AI|Territory")
    int32 TerritorySectors = 8;  // 섹터 수 (8방향)
    
    UPROPERTY(BlueprintReadOnly, Category = "AI|Territory")
    TArray<float> SectorMaxDistances;  // 각 섹터별 최대 거리
    
    UPROPERTY(BlueprintReadOnly, Category = "AI|Territory")
    TArray<FVector> SectorDirections;  // 각 섹터 방향 벡터
    
    UPROPERTY(BlueprintReadOnly, Category = "AI|Territory")
    int32 CurrentSector = -1;  // 현재 위치한 섹터
    
    // Territory Functions
    void InitializeTerritory();  // BeginPlay에서 호출
    int32 GetCurrentSector() const;  // 현재 섹터 계산
    FVector GetRandomPatrolLocationInTerritory();  // 영역 내 배회 위치 생성
    bool IsInTerritory(const FVector& Location) const;  // 영역 내 여부 체크
};