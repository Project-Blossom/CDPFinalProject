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
};