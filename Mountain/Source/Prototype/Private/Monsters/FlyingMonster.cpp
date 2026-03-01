#include "Monsters/FlyingMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

AFlyingMonster::AFlyingMonster()
{
    // Flying Movement Setup
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void AFlyingMonster::BeginPlay()
{
    Super::BeginPlay();

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
    FVector DesiredDirection = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector FinalDirection = DesiredDirection;
    
    // 회피 중이면 회피 방향 유지
    if (bIsAvoiding)
    {
        AvoidanceTimer -= GetWorld()->GetDeltaSeconds();
        
        if (AvoidanceTimer <= 0.0f)
        {
            // 회피 종료 - 다시 체크
            bIsAvoiding = false;
        }
        else
        {
            // 회피 방향 유지
            FinalDirection = AvoidanceDirection;
            
#if !UE_BUILD_SHIPPING
            DrawDebugLine(GetWorld(), CurrentLocation, CurrentLocation + FinalDirection * 200.0f, 
                FColor::Orange, false, 0.1f, 0, 5.0f);
#endif
        }
    }
    else
    {
        // 정상 비행 중 - 장애물 체크
        FVector HitLocation;
        if (CheckForObstacles(DesiredDirection, ObstacleCheckDistance, HitLocation))
        {
            // 장애물 발견 - 회피 시작
            FVector ObstacleNormal = (CurrentLocation - HitLocation).GetSafeNormal();
            AvoidanceDirection = GetAvoidanceDirection(DesiredDirection, ObstacleNormal);
            
            bIsAvoiding = true;
            AvoidanceTimer = AvoidanceDuration;
            FinalDirection = AvoidanceDirection;
            
            UE_LOG(LogMonster, Warning, TEXT("%s: Obstacle detected! Starting avoidance"), *GetName());
            
#if !UE_BUILD_SHIPPING
            DrawDebugLine(GetWorld(), CurrentLocation, HitLocation, FColor::Red, false, 1.0f, 0, 3.0f);
            DrawDebugSphere(GetWorld(), HitLocation, 30.0f, 8, FColor::Red, false, 1.0f);
#endif
        }
    }
    
    // 이동
    FVector NewLocation = CurrentLocation + FinalDirection * Speed * GetWorld()->GetDeltaSeconds();
    SetActorLocation(NewLocation);

    // 목표 방향으로 회전
    if (!FinalDirection.IsNearlyZero())
    {
        FRotator TargetRotation = FinalDirection.Rotation();
        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, 
            GetWorld()->GetDeltaSeconds(), 5.0f);
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
    int32 MaxAttempts = 10;
    
    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            FMath::RandRange(0.0f, VerticalPatrolRange)
        );

        FVector TargetLocation = IdleLocation + RandomOffset;
        TargetLocation.Z = FMath::Max(TargetLocation.Z, 100.0f);
        
        // 이 위치가 암벽 안인지 체크
        if (IsLocationValid(TargetLocation))
        {
            UE_LOG(LogMonster, Verbose, TEXT("%s: Valid patrol location found on attempt %d"), *GetName(), Attempt + 1);
            return TargetLocation;
        }
    }
    
    // 실패하면 IdleLocation 위쪽으로 (안전한 위치)
    UE_LOG(LogMonster, Warning, TEXT("%s: Failed to find valid patrol location, using safe fallback"), *GetName());
    return IdleLocation + FVector(0, 0, 300.0f);
}

bool AFlyingMonster::CheckForObstacles(const FVector& Direction, float Distance, FVector& OutHitLocation)
{
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation + Direction * Distance;
    
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    // 전방 체크
    if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, Params))
    {
        // 너무 가까우면 회피 필요
        if (Hit.Distance < ObstacleAvoidanceDistance)
        {
            OutHitLocation = Hit.Location;
            return true;
        }
    }
    
    return false;
}

FVector AFlyingMonster::GetAvoidanceDirection(const FVector& DesiredDirection, const FVector& ObstacleNormal)
{
    // 장애물에서 멀어지는 방향을 기본으로
    FVector AwayFromObstacle = ObstacleNormal;
    
    // 위쪽 방향 선호 (장애물 위로 넘어가기)
    FVector UpBias = FVector::UpVector;
    
    // 옆으로 회피 (장애물에 수직인 방향)
    FVector RightVector = FVector::CrossProduct(ObstacleNormal, FVector::UpVector).GetSafeNormal();
    
    // 원하는 방향과 가까운 쪽으로 회피
    float DotRight = FVector::DotProduct(DesiredDirection, RightVector);
    FVector SideDirection = (DotRight > 0) ? RightVector : -RightVector;
    
    // 최종 회피 방향: 위 + 옆 + 뒤로
    FVector AvoidDirection = (UpBias * 0.5f + SideDirection * 0.3f + AwayFromObstacle * 0.2f).GetSafeNormal();
    
    // 이 방향에 장애물이 있으면 위로만
    FVector TempHit;
    if (CheckForObstacles(AvoidDirection, ObstacleAvoidanceDistance, TempHit))
    {
        AvoidDirection = (FVector::UpVector * 0.7f + SideDirection * 0.3f).GetSafeNormal();
    }
    
    return AvoidDirection;
}

bool AFlyingMonster::IsLocationValid(const FVector& Location) const
{
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    // 1. 위아래로 암벽이 가까이 있는지 체크 (암벽 안 판정)
    FHitResult HitUp, HitDown;
    FVector TraceUp = Location + FVector::UpVector * 100.0f;
    FVector TraceDown = Location - FVector::UpVector * 100.0f;
    
    bool bHitUp = GetWorld()->LineTraceSingleByChannel(HitUp, Location, TraceUp, ECC_WorldStatic, Params);
    bool bHitDown = GetWorld()->LineTraceSingleByChannel(HitDown, Location, TraceDown, ECC_WorldStatic, Params);
    
    // 위아래 둘 다 가까이 막혀 있으면 암벽 안
    if (bHitUp && bHitDown && HitUp.Distance < 50.0f && HitDown.Distance < 50.0f)
    {
        return false;
    }
    
    // 2. 현재 위치에서 목표 위치까지 경로에 암벽이 있는지 체크
    FVector CurrentLocation = GetActorLocation();
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, CurrentLocation, Location, ECC_WorldStatic, Params))
    {
        // 경로 중간에 암벽 있으면 무효
        return false;
    }
    
    // 3. 목표 위치 주변에 충분한 공간이 있는지 체크
    TArray<FVector> CheckDirections = {
        FVector::ForwardVector,
        -FVector::ForwardVector,
        FVector::RightVector,
        -FVector::RightVector
    };
    
    int32 BlockedCount = 0;
    for (const FVector& Dir : CheckDirections)
    {
        FVector CheckEnd = Location + Dir * 200.0f;
        if (GetWorld()->LineTraceSingleByChannel(Hit, Location, CheckEnd, ECC_WorldStatic, Params))
        {
            if (Hit.Distance < 100.0f)
            {
                BlockedCount++;
            }
        }
    }
    
    // 4방향 중 3개 이상 막혀있으면 너무 좁음
    if (BlockedCount >= 3)
    {
        return false;
    }
    
    return true;
}