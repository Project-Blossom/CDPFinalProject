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

    // Organic Movement (유기적 움직임)
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector CurrentTargetPoint;             // 현재 목표 지점

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float CurrentSpeed = 0.0f;              // 현재 속도

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float TargetSpeed = 200.0f;             // 목표 속도

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsPaused = false;                 // 일시 정지 중

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PauseTimer = 0.0f;                // 정지 타이머

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float NextPauseDuration = 0.0f;         // 다음 정지 시간

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    TArray<FVector> PatrolWaypoints;        // 불규칙 경로 웨이포인트

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    int32 CurrentWaypointIndex = 0;         // 현재 웨이포인트

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WaypointReachThreshold = 50.0f;   // 웨이포인트 도착 거리

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MinSpeed = 50.0f;                 // 최소 속도

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxSpeed = 250.0f;                // 최대 속도

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SpeedChangeRate = 100.0f;         // 속도 변화율

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MinPauseDuration = 0.5f;          // 최소 정지 시간

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxPauseDuration = 3.0f;          // 최대 정지 시간

    UPROPERTY(EditAnywhere, Category = "Movement")
    float PauseChance = 0.1f;               // 정지 확률 (10%)

    // ============================================
    // Attack State
    // ============================================
    UPROPERTY(EditAnywhere, Category = "Attack")
    float DetectionGaugeMax = 100.0f;       // 감지 게이지 최대값

    UPROPERTY(EditAnywhere, Category = "Attack")
    float DetectionGainRate = 20.0f;        // 게이지 증가율 (초당)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float DetectionDecayRate = 10.0f;       // 게이지 감소율 (초당)

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float DetectionGauge = 0.0f;            // 현재 감지 게이지

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    AActor* PotentialTarget = nullptr;      // 감지 중인 대상 (게이지 쌓는 중)
    
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttachRange = 150.0f;              // 달라붙기 범위 (증가)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float StaminaDrainRate = 10.0f;         // Stamina 흡수율 (초당)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float ShakeThreshold = 1000.0f;          // 떨쳐내기 임계값 (증가)

    UPROPERTY(EditAnywhere, Category = "Attack")
    float PursuitSpeed = 300.0f;            // 추격 속도

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    bool bAttachedToPlayer = false;         // 플레이어에 달라붙음

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float AccumulatedShake = 0.0f;          // 누적된 흔들기

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    FVector2D LastMousePosition;            // 이전 마우스 위치

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
    // void CirclePatrol(float DeltaTime);     // 원형 배회 (사용 안 함)
    void OrganicPatrol(float DeltaTime);    // 유기적 배회 (신규)
    void GeneratePatrolWaypoints();         // 불규칙 경로 생성
    void UpdateMovementSpeed(float DeltaTime);  // 속도 업데이트

    // Attack
    UFUNCTION(BlueprintCallable, Category = "Attack")
    void PursuePlayer(float DeltaTime);         // 플레이어 추적

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void AttachToPlayer(class ADownfallCharacter* Player);  // 달라붙기

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void DetachFromPlayer();                    // 떨어지기

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void DrainStamina(float DeltaTime);         // Stamina 흡수

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void UpdateShakeDetection(float DeltaTime); // 흔들기 감지
    
    UFUNCTION(BlueprintCallable, Category = "Attack")
    void UpdateDetectionGauge(float DeltaTime); // 감지 게이지 업데이트

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector ProjectToWallSurface(FVector WorldDirection);  // 벽 표면에 방향 투영

    // Override
    virtual void Attack() override {}       // Phase 2에서 구현
    virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;  // 게이지 방식
};