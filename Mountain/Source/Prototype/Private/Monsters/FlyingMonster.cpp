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
	// IdleLocation 주변 3D 구형 범위 내 랜덤 위치
	// 수평: PatrolRadius 범위
	// 수직: IdleLocation.Z 기준 +1000 범위
    
	FVector RandomOffset = FVector(
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		FMath::RandRange(0.0f, VerticalPatrolRange)
	);

	FVector TargetLocation = IdleLocation + RandomOffset;
    
	// 최소 높이 제한 (지면 아래로 안 가게)
	TargetLocation.Z = FMath::Max(TargetLocation.Z, 100.0f);

	return TargetLocation;
}