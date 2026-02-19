#include "Monsters/WallCrawler.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

AWallCrawler::AWallCrawler()
{
    // Flying 모드로 설정 (벽면 이동용)
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void AWallCrawler::BeginPlay()
{
    Super::BeginPlay();

    // 시작 위치에서 벽 찾기
    FVector WallNormal;
    FVector HitLocation;
    if (DetectWall(WallNormal, HitLocation))
    {
        CurrentWallNormal = WallNormal;
        WallHitLocation = HitLocation;
        bIsOnWall = true;
        PatrolCenter = GetActorLocation();
        
        // 불규칙 경로 생성
        GeneratePatrolWaypoints();
        
        // 초기 속도 설정
        CurrentSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
        TargetSpeed = CurrentSpeed;
        
        UE_LOG(LogMonster, Log, TEXT("%s found wall, normal: %s"), *GetName(), *WallNormal.ToString());
    }
    else
    {
        UE_LOG(LogMonster, Warning, TEXT("%s NOT on wall at start!"), *GetName());
    }
}

void AWallCrawler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 벽면 정렬 업데이트
    UpdateWallAlignment();

    // 유기적 배회
    if (bIsOnWall)
    {
        OrganicPatrol(DeltaTime);
    }

#if !UE_BUILD_SHIPPING
    // 디버그 시각화
    FVector ActorLoc = GetActorLocation();
    
    // 벽 Normal 표시 (파란 화살표)
    if (bIsOnWall)
    {
        DrawDebugLine(
            GetWorld(),
            ActorLoc,
            ActorLoc + CurrentWallNormal * 100.0f,
            FColor::Blue,
            false,
            0.1f,
            0,
            3.0f
        );
        
        // 벽 접촉점 (녹색 구체)
        DrawDebugSphere(
            GetWorld(),
            WallHitLocation,
            10.0f,
            8,
            FColor::Green,
            false,
            0.1f,
            0,
            2.0f
        );
        
        // 배회 경로 (벽면 투영된 노란 원)
        const int32 NumSegments = 32;
        for (int32 i = 0; i < NumSegments; i++)
        {
            float Angle1 = (float)i / NumSegments * 2.0f * PI;
            float Angle2 = (float)(i + 1) / NumSegments * 2.0f * PI;
            
            // 벽 Normal에 수직인 두 축
            FVector Up = FVector::UpVector;
            FVector Right = FVector::CrossProduct(CurrentWallNormal, Up).GetSafeNormal();
            FVector ActualUp = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
            
            // 원의 두 점 (벽 표면 좌표계)
            FVector Offset1 = Right * FMath::Cos(Angle1) * CircleRadius + ActualUp * FMath::Sin(Angle1) * CircleRadius;
            FVector Offset2 = Right * FMath::Cos(Angle2) * CircleRadius + ActualUp * FMath::Sin(Angle2) * CircleRadius;
            
            FVector Point1 = PatrolCenter + Offset1;
            FVector Point2 = PatrolCenter + Offset2;
            
            // 각 점을 벽면에 투영
            FHitResult Hit1, Hit2;
            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(this);
            
            // Point1을 벽에 투영
            FVector TraceStart1 = Point1 - CurrentWallNormal * 50.0f;
            FVector TraceEnd1 = Point1 + CurrentWallNormal * 50.0f;
            if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart1, TraceEnd1, ECC_Visibility, TraceParams))
            {
                Point1 = Hit1.Location + Hit1.Normal * 5.0f;  // 벽에서 약간 떨어진 위치
            }
            
            // Point2를 벽에 투영
            FVector TraceStart2 = Point2 - CurrentWallNormal * 50.0f;
            FVector TraceEnd2 = Point2 + CurrentWallNormal * 50.0f;
            if (GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart2, TraceEnd2, ECC_Visibility, TraceParams))
            {
                Point2 = Hit2.Location + Hit2.Normal * 5.0f;
            }
            
            // 선 그리기
            DrawDebugLine(
                GetWorld(),
                Point1,
                Point2,
                FColor::Yellow,
                false,
                0.1f,
                0,
                2.0f
            );
        }
    }

    // 상태 텍스트
    DrawDebugString(
        GetWorld(),
        ActorLoc + FVector(0, 0, 50),
        bIsOnWall ? TEXT("ON WALL") : TEXT("NO WALL"),
        nullptr,
        bIsOnWall ? FColor::Green : FColor::Red,
        0.0f,
        true
    );
#endif
}

bool AWallCrawler::DetectWall(FVector& OutWallNormal, FVector& OutHitLocation)
{
    FVector ActorLoc = GetActorLocation();
    FVector ActorForward = GetActorForwardVector();
    
    // 전방으로 Ray 발사
    FVector TraceStart = ActorLoc;
    FVector TraceEnd = ActorLoc + ActorForward * WallTraceDistance;
    
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        OutWallNormal = Hit.Normal;
        OutHitLocation = Hit.Location;
        return true;
    }
    
    return false;
}

void AWallCrawler::UpdateWallAlignment()
{
    // 매 프레임 벽 재감지
    FVector WallNormal;
    FVector HitLocation;
    
    if (DetectWall(WallNormal, HitLocation))
    {
        CurrentWallNormal = WallNormal;
        WallHitLocation = HitLocation;
        bIsOnWall = true;
        
        // 벽에 수직으로 회전
        FRotator TargetRotation = (-WallNormal).Rotation();
        FRotator NewRotation = FMath::RInterpTo(
            GetActorRotation(),
            TargetRotation,
            GetWorld()->GetDeltaSeconds(),
            5.0f
        );
        SetActorRotation(NewRotation);
        
        // 벽에서 일정 거리 유지
        FVector TargetLocation = HitLocation + WallNormal * WallStickDistance;
        SetActorLocation(TargetLocation);
    }
    else
    {
        bIsOnWall = false;
        UE_LOG(LogMonster, Warning, TEXT("%s lost wall!"), *GetName());
    }
}

void AWallCrawler::CrawlOnWall(FVector Direction, float Speed)
{
    if (!bIsOnWall)
        return;
    
    // 방향을 벽 표면에 투영
    FVector ProjectedDirection = ProjectToWallSurface(Direction);
    
    // 이동
    FVector NewLocation = GetActorLocation() + ProjectedDirection * Speed * GetWorld()->GetDeltaSeconds();
    SetActorLocation(NewLocation);
}

void AWallCrawler::CirclePatrol(float DeltaTime)
{
    // 각도 증가
    PatrolAngle += PatrolAngularSpeed * DeltaTime;
    if (PatrolAngle >= 360.0f)
        PatrolAngle -= 360.0f;
    
    // 원형 경로 계산 (벽 표면 기준)
    float AngleRad = FMath::DegreesToRadians(PatrolAngle);
    
    // 벽 Normal에 수직인 두 축 계산
    FVector Up = FVector::UpVector;
    FVector Right = FVector::CrossProduct(CurrentWallNormal, Up).GetSafeNormal();
    FVector ActualUp = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
    
    // 원형 오프셋
    FVector CircleOffset = Right * FMath::Cos(AngleRad) * CircleRadius 
                         + ActualUp * FMath::Sin(AngleRad) * CircleRadius;
    
    // 목표 위치
    FVector TargetLocation = PatrolCenter + CircleOffset;
    
    // 목표 방향
    FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
    
    // 이동
    CrawlOnWall(Direction, CrawlSpeed);
}

FVector AWallCrawler::ProjectToWallSurface(FVector WorldDirection)
{
    if (!bIsOnWall)
        return WorldDirection;
    
    // 벽 Normal에 수직인 성분만 남김
    FVector ProjectedDirection = WorldDirection - CurrentWallNormal * FVector::DotProduct(WorldDirection, CurrentWallNormal);
    return ProjectedDirection.GetSafeNormal();
}

// ============================================
// Organic Patrol (유기적 배회)
// ============================================

void AWallCrawler::GeneratePatrolWaypoints()
{
    PatrolWaypoints.Empty();
    
    // 8-12개의 불규칙한 웨이포인트 생성
    int32 NumWaypoints = FMath::RandRange(8, 12);
    
    for (int32 i = 0; i < NumWaypoints; i++)
    {
        float Angle = (float)i / NumWaypoints * 2.0f * PI;
        
        // 반경을 랜덤하게 변형 (50% ~ 150%)
        float RadiusMod = FMath::RandRange(0.5f, 1.5f);
        float ActualRadius = CircleRadius * RadiusMod;
        
        // 각도도 약간 랜덤하게
        float AngleOffset = FMath::RandRange(-0.3f, 0.3f);
        float ActualAngle = Angle + AngleOffset;
        
        // 벽 Normal에 수직인 두 축
        FVector Up = FVector::UpVector;
        FVector Right = FVector::CrossProduct(CurrentWallNormal, Up).GetSafeNormal();
        FVector ActualUp = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
        
        // 웨이포인트 계산
        FVector Offset = Right * FMath::Cos(ActualAngle) * ActualRadius 
                       + ActualUp * FMath::Sin(ActualAngle) * ActualRadius;
        
        FVector Waypoint = PatrolCenter + Offset;
        
        // 벽면에 투영
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        FVector TraceStart = Waypoint - CurrentWallNormal * 100.0f;
        FVector TraceEnd = Waypoint + CurrentWallNormal * 100.0f;
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            Waypoint = Hit.Location + Hit.Normal * WallStickDistance;
        }
        
        PatrolWaypoints.Add(Waypoint);
    }
    
    CurrentWaypointIndex = 0;
    if (PatrolWaypoints.Num() > 0)
    {
        CurrentTargetPoint = PatrolWaypoints[0];
    }
    
    UE_LOG(LogMonster, Log, TEXT("%s generated %d waypoints"), *GetName(), PatrolWaypoints.Num());
}

void AWallCrawler::OrganicPatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0)
        return;
    
    // 일시 정지 처리
    if (bIsPaused)
    {
        PauseTimer += DeltaTime;
        CurrentSpeed = 0.0f;
        
        if (PauseTimer >= NextPauseDuration)
        {
            bIsPaused = false;
            PauseTimer = 0.0f;
            TargetSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
            UE_LOG(LogMonster, Verbose, TEXT("%s resumed movement"), *GetName());
        }
        return;
    }
    
    // 속도 업데이트
    UpdateMovementSpeed(DeltaTime);
    
    // 현재 웨이포인트로 이동
    FVector ToTarget = CurrentTargetPoint - GetActorLocation();
    float DistanceToTarget = ToTarget.Size();
    
    if (DistanceToTarget < WaypointReachThreshold)
    {
        // 웨이포인트 도착!
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        CurrentTargetPoint = PatrolWaypoints[CurrentWaypointIndex];
        
        // 랜덤 정지 확률
        if (FMath::FRand() < PauseChance)
        {
            bIsPaused = true;
            NextPauseDuration = FMath::RandRange(MinPauseDuration, MaxPauseDuration);
            UE_LOG(LogMonster, Verbose, TEXT("%s paused for %.1fs"), *GetName(), NextPauseDuration);
        }
        
        // 새 목표 속도
        TargetSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
        
        // 모든 웨이포인트 순회 시 새 경로 생성 (20% 확률)
        if (CurrentWaypointIndex == 0 && FMath::FRand() < 0.2f)
        {
            GeneratePatrolWaypoints();
            UE_LOG(LogMonster, Log, TEXT("%s regenerated patrol path"), *GetName());
        }
    }
    
    // 이동
    FVector Direction = ToTarget.GetSafeNormal();
    CrawlOnWall(Direction, CurrentSpeed);
}

void AWallCrawler::UpdateMovementSpeed(float DeltaTime)
{
    // 목표 속도로 부드럽게 변화
    if (FMath::Abs(CurrentSpeed - TargetSpeed) > 1.0f)
    {
        if (CurrentSpeed < TargetSpeed)
        {
            CurrentSpeed = FMath::Min(CurrentSpeed + SpeedChangeRate * DeltaTime, TargetSpeed);
        }
        else
        {
            CurrentSpeed = FMath::Max(CurrentSpeed - SpeedChangeRate * DeltaTime, TargetSpeed);
        }
    }
}