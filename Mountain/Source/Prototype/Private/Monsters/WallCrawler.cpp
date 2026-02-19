#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"
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

    // 행동 우선순위
    if (bAttachedToPlayer)
    {
        // 달라붙은 상태 - Stamina 흡수
        DrainStamina(DeltaTime);
        UpdateShakeDetection(DeltaTime);
    }
    else if (bIsOnWall)
    {
        if (TargetPlayer)
        {
            // 플레이어 감지 - 추격
            float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
            
            if (Distance <= AttachRange && TargetPlayer->GetCharacterMovement()->MovementMode == MOVE_Flying)
            {
                // 범위 내 + 플레이어 등반 중 → 달라붙기!
                AttachToPlayer(TargetPlayer);
            }
            else
            {
                // 추격
                PursuePlayer(DeltaTime);
            }
        }
        else
        {
            // 유기적 배회
            OrganicPatrol(DeltaTime);
        }
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
        
        // 배회 경로 (웨이포인트 연결)
        if (PatrolWaypoints.Num() > 0)
        {
            for (int32 i = 0; i < PatrolWaypoints.Num(); i++)
            {
                int32 NextIndex = (i + 1) % PatrolWaypoints.Num();
                
                // 웨이포인트 연결 선
                DrawDebugLine(
                    GetWorld(),
                    PatrolWaypoints[i],
                    PatrolWaypoints[NextIndex],
                    FColor::Yellow,
                    false,
                    0.1f,
                    0,
                    2.0f
                );
                
                // 웨이포인트 표시 (작은 구체)
                FColor WaypointColor = (i == CurrentWaypointIndex) ? FColor::Orange : FColor::Yellow;
                DrawDebugSphere(
                    GetWorld(),
                    PatrolWaypoints[i],
                    8.0f,
                    6,
                    WaypointColor,
                    false,
                    0.1f,
                    0,
                    1.0f
                );
            }
            
            // 현재 목표로 향하는 선 (빨간색)
            DrawDebugLine(
                GetWorld(),
                ActorLoc,
                CurrentTargetPoint,
                FColor::Red,
                false,
                0.1f,
                0,
                3.0f
            );
        }
    }

    // AI Perception 감지 범위 (노란 구체)
    DrawDebugSphere(
        GetWorld(),
        ActorLoc,
        SightRadius,
        16,
        FColor::Yellow,
        false,
        0.1f,
        0,
        1.0f
    );
    
    // 시야각 표시 (원뿔)
    // Perception의 실제 Forward 방향 사용 (벽 Normal)
    FVector ForwardDir = bIsOnWall ? CurrentWallNormal : GetActorForwardVector();
    float HalfAngleRad = FMath::DegreesToRadians(SightAngle);
    
    // 왼쪽 경계
    FVector LeftDir = ForwardDir.RotateAngleAxis(-SightAngle, FVector::UpVector);
    DrawDebugLine(
        GetWorld(),
        ActorLoc,
        ActorLoc + LeftDir * SightRadius,
        FColor::Cyan,
        false,
        0.1f,
        0,
        2.0f
    );
    
    // 오른쪽 경계
    FVector RightDir = ForwardDir.RotateAngleAxis(SightAngle, FVector::UpVector);
    DrawDebugLine(
        GetWorld(),
        ActorLoc,
        ActorLoc + RightDir * SightRadius,
        FColor::Cyan,
        false,
        0.1f,
        0,
        2.0f
    );
    
    // 중앙선
    DrawDebugLine(
        GetWorld(),
        ActorLoc,
        ActorLoc + ForwardDir * SightRadius,
        FColor::Green,
        false,
        0.1f,
        0,
        3.0f
    );
    
    // 상태 텍스트
    FString StateText = bAttachedToPlayer ? TEXT("ATTACHED") : (bIsOnWall ? TEXT("ON WALL") : TEXT("NO WALL"));
    FColor StateColor = bAttachedToPlayer ? FColor::Red : (bIsOnWall ? FColor::Green : FColor::Red);
    
    DrawDebugString(
        GetWorld(),
        ActorLoc + FVector(0, 0, 50),
        StateText,
        nullptr,
        StateColor,
        0.0f,
        true
    );
    
    // TargetPlayer 상태
    DrawDebugString(
        GetWorld(),
        ActorLoc + FVector(0, 0, 70),
        TargetPlayer ? FString::Printf(TEXT("TARGET: %.0fcm"), FVector::Dist(ActorLoc, TargetPlayer->GetActorLocation())) : TEXT("NO TARGET"),
        nullptr,
        TargetPlayer ? FColor::Red : FColor::White,
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
    
    // ECC_WorldStatic으로 변경 (더 넓은 범위)
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params);
    
#if !UE_BUILD_SHIPPING
    // 디버그: Trace 시각화
    DrawDebugLine(
        GetWorld(),
        TraceStart,
        TraceEnd,
        bHit ? FColor::Green : FColor::Red,
        false,
        0.1f,
        0,
        5.0f
    );
    
    if (bHit)
    {
        DrawDebugPoint(GetWorld(), Hit.Location, 10.0f, FColor::Yellow, false, 0.1f, 0);
        
        // Normal 표시
        DrawDebugLine(
            GetWorld(),
            Hit.Location,
            Hit.Location + Hit.Normal * 50.0f,
            FColor::Cyan,
            false,
            0.1f,
            0,
            3.0f
        );
    }
#endif
    
    if (bHit)
    {
        OutWallNormal = Hit.Normal;
        OutHitLocation = Hit.Location;
        UE_LOG(LogMonster, Verbose, TEXT("%s DetectWall HIT: %s, Normal: %s"), 
            *GetName(), 
            Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("NULL"), 
            *Hit.Normal.ToString());
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
        
        // 원래대로: 벽에 수직으로 회전 (벽 안쪽 향함)
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

// ============================================
// Attack Functions (공격)
// ============================================

void AWallCrawler::PursuePlayer(float DeltaTime)
{
    if (!TargetPlayer)
        return;
    
    // 플레이어 위치를 벽 표면에 투영
    FVector PlayerLocation = TargetPlayer->GetActorLocation();
    FVector ToPlayer = PlayerLocation - GetActorLocation();
    
    // 벽 표면에 투영된 방향
    FVector ProjectedDirection = ProjectToWallSurface(ToPlayer.GetSafeNormal());
    
    // 추격
    CrawlOnWall(ProjectedDirection, PursuitSpeed);
    
#if !UE_BUILD_SHIPPING
    // 디버그: 추격 선 (빨간색)
    DrawDebugLine(
        GetWorld(),
        GetActorLocation(),
        PlayerLocation,
        FColor::Red,
        false,
        0.1f,
        0,
        3.0f
    );
#endif
}

void AWallCrawler::AttachToPlayer(ADownfallCharacter* Player)
{
    if (!Player || bAttachedToPlayer)
        return;
    
    bAttachedToPlayer = true;
    TargetPlayer = Player;
    AccumulatedShake = 0.0f;
    
    // 플레이어에게 부착
    AttachToActor(Player, FAttachmentTransformRules::KeepWorldTransform);
    
    // 상대 위치 조정 (플레이어 등 뒤)
    FVector RelativeLocation = FVector(0, 0, -30.0f);  // 약간 아래쪽
    SetActorRelativeLocation(RelativeLocation);
    
    UE_LOG(LogMonster, Warning, TEXT("%s ATTACHED to player!"), *GetName());
}

void AWallCrawler::DetachFromPlayer()
{
    if (!bAttachedToPlayer)
        return;
    
    bAttachedToPlayer = false;
    AccumulatedShake = 0.0f;
    
    // 플레이어에서 떨어짐
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
    // 약간 뒤로 밀려남
    FVector PushDirection = -GetActorForwardVector();
    FVector NewLocation = GetActorLocation() + PushDirection * 100.0f;
    SetActorLocation(NewLocation);
    
    UE_LOG(LogMonster, Warning, TEXT("%s DETACHED from player!"), *GetName());
}

void AWallCrawler::DrainStamina(float DeltaTime)
{
    if (!TargetPlayer || !bAttachedToPlayer)
        return;
    
    // Stamina 감소
    float DrainAmount = StaminaDrainRate * DeltaTime;
    
    // DownfallCharacter의 Stamina 직접 감소
    if (TargetPlayer->LeftHand.State == EHandState::Gripping)
    {
        TargetPlayer->LeftHand.Stamina = FMath::Max(0.0f, TargetPlayer->LeftHand.Stamina - DrainAmount);
    }
    if (TargetPlayer->RightHand.State == EHandState::Gripping)
    {
        TargetPlayer->RightHand.Stamina = FMath::Max(0.0f, TargetPlayer->RightHand.Stamina - DrainAmount);
    }
}

void AWallCrawler::UpdateShakeDetection(float DeltaTime)
{
    if (!TargetPlayer || !bAttachedToPlayer)
        return;
    
    // 플레이어 컨트롤러 가져오기
    APlayerController* PC = Cast<APlayerController>(TargetPlayer->GetController());
    if (!PC)
        return;
    
    // 현재 마우스 위치
    float MouseX, MouseY;
    PC->GetMousePosition(MouseX, MouseY);
    FVector2D CurrentMousePosition(MouseX, MouseY);
    
    // 첫 프레임
    if (LastMousePosition.IsNearlyZero())
    {
        LastMousePosition = CurrentMousePosition;
        return;
    }
    
    // 마우스 이동량 계산
    FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;
    float MouseMovement = MouseDelta.Size();
    
    // 누적
    AccumulatedShake += MouseMovement;
    
    // 임계값 도달 시 떨어짐
    if (AccumulatedShake >= ShakeThreshold)
    {
        UE_LOG(LogMonster, Warning, TEXT("%s shaken off! (%.1f accumulated)"), *GetName(), AccumulatedShake);
        DetachFromPlayer();
    }
    
    // 마우스 위치 업데이트
    LastMousePosition = CurrentMousePosition;
    
#if !UE_BUILD_SHIPPING
    // 디버그: 흔들기 게이지
    if (GEngine)
    {
        FString ShakeText = FString::Printf(TEXT("Shake: %.0f / %.0f"), AccumulatedShake, ShakeThreshold);
        GEngine->AddOnScreenDebugMessage(
            -1,
            0.0f,
            AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow,
            ShakeText
        );
    }
#endif
}