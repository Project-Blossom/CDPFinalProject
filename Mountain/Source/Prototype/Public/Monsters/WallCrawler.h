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
    
    // Wall Movement
    UPROPERTY(EditAnywhere, Category = "Movement")
    float CrawlSpeed = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float CircleRadius = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WallTraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WallStickDistance = 20.0f;
    
    // Wall State
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector CurrentWallNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsOnWall = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector WallHitLocation;
    
    // Patrol State
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PatrolAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PatrolAngularSpeed = 30.0f;

    // Organic Movement
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector CurrentTargetPoint;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float TargetSpeed = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsPaused = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float PauseTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float NextPauseDuration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WaypointReachThreshold = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MinSpeed = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxSpeed = 250.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SpeedChangeRate = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MinPauseDuration = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxPauseDuration = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float PauseChance = 0.1f;
    
    // Attack State
    // DetectionGaugeMax, DetectionGainRate, DetectionDecayRate, DetectionGauge, PotentialTarget는 MonsterBase로 이동
    
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttachRange = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float StaminaDrainRate = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float ShakeThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float PursuitSpeed = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    bool bAttachedToPlayer = false;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float AccumulatedShake = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    FVector2D LastMousePosition;
    
    // Detach & Stun System
    UPROPERTY(EditAnywhere, Category = "Attack")
    float StunDuration = 3.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    bool bIsStunned = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float StunTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float GravityScale = 1.0f;
    
    // 스폰 직후 Attach 방지
    float SpawnTime = 0.0f;
    float MinTimeBeforeAttach = 2.0f;


    // Functions
    // Wall Detection
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool DetectWall(FVector& OutWallNormal, FVector& OutHitLocation);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateWallAlignment();

    // Movement
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void CrawlOnWall(FVector Direction, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void OrganicPatrol(float DeltaTime);
    void GeneratePatrolWaypoints();
    void UpdateMovementSpeed(float DeltaTime);

    // Gravity
    void ApplyGravity(float DeltaTime);
    
    // Attack
    UFUNCTION(BlueprintCallable, Category = "Attack")
    void PursuePlayer(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void AttachToPlayer(class ADownfallCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void DetachFromPlayer();

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void DrainStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void UpdateShakeDetection(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Attack")
    void UpdateDetectionGauge(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector ProjectToWallSurface(FVector WorldDirection);

    // Carrier System (FlyingPlatform)
    UPROPERTY(BlueprintReadOnly, Category = "Carrier")
    bool bIsCarried = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Carrier")
    bool bIsFalling = false;

    UPROPERTY()
    class AFlyingPlatform* CarrierPlatform = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Carrier")
    void AttachToCarrier(class AFlyingPlatform* Platform);

    UFUNCTION(BlueprintCallable, Category = "Carrier")
    void DetachFromCarrier();

    void HandleLanding();  // 착지 처리 (OnCarrierLanded 대체)

    // Override
    virtual void Attack() override {}
    virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;
};