#include "Monsters/FlyingMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

AFlyingMonster::AFlyingMonster()
{
	// Flying Movement Setup
	GetCharacterMovement()->GravityScale = 0.0f;  // 중력 없음
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void AFlyingMonster::BeginPlay()
{
	Super::BeginPlay();

	// 시작 위치를 Idle 위치로 설정
	IdleLocation = GetActorLocation();
    
	UE_LOG(LogMonster, Log, TEXT("%s idle location set: %s"), *GetName(), *IdleLocation.ToString());
}

void AFlyingMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFlyingMonster::FlyToLocation(FVector TargetLocation, float Speed)
{
	FVector CurrentLocation = GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
	// 이동
	FVector NewLocation = CurrentLocation + Direction * Speed * GetWorld()->GetDeltaSeconds();
	SetActorLocation(NewLocation);

	// 목표 방향으로 회전
	if (!Direction.IsNearlyZero())
	{
		FRotator TargetRotation = Direction.Rotation();
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 5.0f);
		SetActorRotation(NewRotation);
	}
}

void AFlyingMonster::PatrolRandomly()
{
	bIsIdling = false;
	FVector TargetLocation = GetRandomPatrolLocation();
	FlyToLocation(TargetLocation, FlightSpeed);
}

void AFlyingMonster::ReturnToIdle()
{
	bIsIdling = true;
	FlyToLocation(IdleLocation, FlightSpeed * 0.5f);
}

FVector AFlyingMonster::GetRandomPatrolLocation() const
{
	// IdleLocation 주변 PatrolRadius 내 랜덤 위치
	FVector RandomOffset = FVector(
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		FMath::RandRange(-FlightHeight * 0.5f, FlightHeight * 0.5f)
	);

	return IdleLocation + RandomOffset;
}