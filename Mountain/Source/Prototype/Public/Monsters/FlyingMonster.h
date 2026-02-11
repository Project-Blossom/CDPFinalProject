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

	// ============================================
	// Flight Settings
	// ============================================
	UPROPERTY(EditAnywhere, Category = "Flight")
	float FlightSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Flight")
	float FlightHeight = 200.0f;        // 지면으로부터 높이

	UPROPERTY(EditAnywhere, Category = "Flight")
	float PatrolRadius = 500.0f;        // 배회 반경

	// ============================================
	// Idle Location (암벽 근처 "둥지")
	// ============================================
	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	FVector IdleLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Flight")
	bool bIsIdling = true;

	// ============================================
	// Functions
	// ============================================
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void FlyToLocation(FVector TargetLocation, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Flight")
	void PatrolRandomly();

	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ReturnToIdle();

	UFUNCTION(BlueprintPure, Category = "Flight")
	FVector GetRandomPatrolLocation() const;
};