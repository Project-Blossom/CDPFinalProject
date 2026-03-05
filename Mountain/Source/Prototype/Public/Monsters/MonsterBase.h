#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "MonsterBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMonster, Log, All);

UCLASS(Abstract)
class PROTOTYPE_API AMonsterBase : public ACharacter
{
    GENERATED_BODY()

public:
    AMonsterBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class ADownfallCharacter* TargetPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* PotentialTarget = nullptr;
    
    // Perception Settings
    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float SightRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float SightAngle = 60.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float LoseSightRadius = 1500.0f;

    // Detection Gauge
    UPROPERTY(BlueprintReadOnly, Category = "AI|Detection")
    float DetectionGauge = 0.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Detection")
    float DetectionGaugeMax = 100.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Detection")
    float DetectionGainRate = 20.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Detection")
    float DetectionDecayRate = 10.0f;
    
    // Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;
    
    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebug = false;

    // Functions
    UFUNCTION()
    virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void TakeDamageCustom(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void Die();

    UFUNCTION(BlueprintPure, Category = "Monster")
    bool IsPlayerDetected() const { return TargetPlayer != nullptr; }
    
    UFUNCTION(BlueprintPure, Category = "Monster")
    float GetDetectionGauge() const { return DetectionGauge; }

    // 순수 가상 함수 (자식 클래스에서 필수 구현)
    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void Attack() PURE_VIRTUAL(AMonsterBase::Attack, );
};