#pragma once

#include "CoreMinimal.h"
#include "Monsters/MonsterBase.h"
#include "FlyingMonster.generated.h"

UCLASS(Abstract)
class PROTOTYPE_API AFlyingMonster : public AMonsterBase
{
	GENERATED_BODY()

public:
	AFlyingMonster();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
    
	// Flight Settings
	UPROPERTY(EditAnywhere, Category = "Flight")
	float FlightSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Flight")
	float FlightHeight = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Flight")
	float PatrolRadius = 500.0f;
    
	UPROPERTY(EditAnywhere, Category = "Flight")
	float VerticalPatrolRange = 1000.0f;
    
	// Obstacle Avoidance
	UPROPERTY(EditAnywhere, Category = "Flight")
	float ObstacleAvoidanceDistance = 300.0f;  // 장애물 감지 시작 거리
    
	UPROPERTY(EditAnywhere, Category = "Flight")
	float ObstacleCheckDistance = 500.0f;      // 전방 체크 거리
    
	// Idle Location 
	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	FVector IdleLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	bool bIsIdling = true;
	
	// Avoidance State
	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	bool bIsAvoiding = false;
    
	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	FVector AvoidanceDirection;
    
	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	float AvoidanceTimer = 0.0f;
    
	UPROPERTY(EditAnywhere, Category = "Flight")
	float AvoidanceDuration = 1.0f;  // 회피 지속 시간
	
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void FlyToLocation(FVector TargetLocation, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Flight")
	void PatrolRandomly();

	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ReturnToIdle();

	UFUNCTION(BlueprintPure, Category = "Flight")
	FVector GetRandomPatrolLocation() const;
	
	UFUNCTION(BlueprintPure, Category = "Flight")
	bool IsLocationValid(const FVector& Location) const;

private:
	// Obstacle Avoidance
	bool CheckForObstacles(const FVector& Direction, float Distance, FVector& OutHitLocation);
	FVector GetAvoidanceDirection(const FVector& DesiredDirection, const FVector& ObstacleNormal);
};