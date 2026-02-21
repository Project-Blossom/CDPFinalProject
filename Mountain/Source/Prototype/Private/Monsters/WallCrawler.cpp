#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Perception/AISenseConfig_Hearing.h"

AWallCrawler::AWallCrawler()
{
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    
    // Collision 설정 - 플레이어와 Overlap만 감지 (밀리지 않음)
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AWallCrawler::BeginPlay()
{
    Super::BeginPlay();
    
    // CRITICAL: WallCrawler는 Hearing만 사용 (방향 무관)
    if (PerceptionComponent)
    {
        // Hearing Config 생성 및 설정
        UAISenseConfig_Hearing* HearingConfig = NewObject<UAISenseConfig_Hearing>(this);
        HearingConfig->HearingRange = 1000.0f;  // 10m
        HearingConfig->SetMaxAge(0.2f);  // 0.2초로 단축 (즉시 반응)
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        
        // Hearing을 재설정하고 Dominant로 강제
        PerceptionComponent->ConfigureSense(*HearingConfig);
        PerceptionComponent->SetDominantSense(HearingConfig->GetSenseImplementation());
        
        UE_LOG(LogMonster, Warning, TEXT("%s configured Hearing-only (Range: 1000cm)"), *GetName());
    }

    FVector WallNormal, HitLocation;
    if (DetectWall(WallNormal, HitLocation))
    {
        CurrentWallNormal = WallNormal;
        WallHitLocation = HitLocation;
        bIsOnWall = true;
        PatrolCenter = GetActorLocation();
        
        GeneratePatrolWaypoints();
        
        CurrentSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
        TargetSpeed = CurrentSpeed;
    }
}

void AWallCrawler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateWallAlignment();
    
    // 감지 게이지 업데이트
    UpdateDetectionGauge(DeltaTime);

    if (bAttachedToPlayer)
    {
        DrainStamina(DeltaTime);
        UpdateShakeDetection(DeltaTime);
    }
    else if (bIsOnWall)
    {
        if (TargetPlayer)
        {
            float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
            
            // CRITICAL: 범위 체크 (1500cm = 15m, Hearing Range보다 50% 더 멀리)
            if (Distance > 1500.0f)
            {
                // 너무 멀어짐 - Target 해제
                TargetPlayer = nullptr;
                UE_LOG(LogMonster, Warning, TEXT("%s lost player (too far: %.1f)"), *GetName(), Distance);
            }
            else if (Distance <= AttachRange)
            {
                // 범위 내 도달 → 달라붙기!
                AttachToPlayer(TargetPlayer);
            }
            else
            {
                PursuePlayer(DeltaTime);
            }
        }
        else
        {
            OrganicPatrol(DeltaTime);
        }
    }

#if !UE_BUILD_SHIPPING
    // 달라붙었을 때는 디버그 표시 안 함
    if (!bAttachedToPlayer)
    {
        FVector ActorLoc = GetActorLocation();
    
    /* === 디버그 시각화 (필요시 주석 해제) === */
    
    /* 벽 Normal & 접촉점
    if (bIsOnWall)
    {
        DrawDebugLine(GetWorld(), ActorLoc, ActorLoc + CurrentWallNormal * 100.0f, FColor::Blue, false, 0.1f, 0, 3.0f);
        DrawDebugSphere(GetWorld(), WallHitLocation, 10.0f, 8, FColor::Green, false, 0.1f, 0, 2.0f);
    }
    */
    
    /* 배회 경로
    if (PatrolWaypoints.Num() > 0)
    {
        for (int32 i = 0; i < PatrolWaypoints.Num(); i++)
        {
            int32 NextIndex = (i + 1) % PatrolWaypoints.Num();
            DrawDebugLine(GetWorld(), PatrolWaypoints[i], PatrolWaypoints[NextIndex], FColor::Yellow, false, 0.1f, 0, 2.0f);
            DrawDebugSphere(GetWorld(), PatrolWaypoints[i], 8.0f, 6, 
                (i == CurrentWaypointIndex) ? FColor::Orange : FColor::Yellow, false, 0.1f, 0, 1.0f);
        }
        DrawDebugLine(GetWorld(), ActorLoc, CurrentTargetPoint, FColor::Red, false, 0.1f, 0, 3.0f);
    }
    */
    
    /* 추격 선
    if (TargetPlayer)
    {
        DrawDebugLine(GetWorld(), ActorLoc, TargetPlayer->GetActorLocation(), FColor::Red, false, 0.1f, 0, 3.0f);
    }
    */
    
    // AI 감지 범위
    DrawDebugSphere(GetWorld(), ActorLoc, 1000.0f, 16, FColor::Yellow, false, 0.1f, 0, 1.0f);
    
    // 상태 텍스트
    DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 50), 
        bAttachedToPlayer ? TEXT("ATTACHED") : (bIsOnWall ? TEXT("ON WALL") : TEXT("NO WALL")),
        nullptr, bAttachedToPlayer ? FColor::Red : (bIsOnWall ? FColor::Green : FColor::Red), 0.0f, true);
    
    // Detection Gauge (감지 게이지)
    if (DetectionGauge > 0.0f || PotentialTarget)
    {
        FColor GaugeColor = DetectionGauge >= DetectionGaugeMax ? FColor::Red : 
                           DetectionGauge > DetectionGaugeMax * 0.5f ? FColor::Orange : FColor::Yellow;
        
        DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 90), 
            FString::Printf(TEXT("Gauge: %.0f/%.0f"), DetectionGauge, DetectionGaugeMax), 
            nullptr, GaugeColor, 0.0f, true);
    }
    
    // TargetPlayer 상태
    if (TargetPlayer)
    {
        DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 70), 
            FString::Printf(TEXT("TARGET: %.0fcm"), FVector::Dist(ActorLoc, TargetPlayer->GetActorLocation())), 
            nullptr, FColor::Red, 0.0f, true);
    }
    }  // if (!bAttachedToPlayer)
    else
    {
        // 달라붙었을 때는 Shake 게이지만 표시
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, 
                AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow,
                FString::Printf(TEXT("WallCrawler Attached! Shake: %.0f / %.0f"), AccumulatedShake, ShakeThreshold));
        }
    }
#endif
}

bool AWallCrawler::DetectWall(FVector& OutWallNormal, FVector& OutHitLocation)
{
    FVector ActorLoc = GetActorLocation();
    FVector ActorForward = GetActorForwardVector();
    FVector ActorUp = GetActorUpVector();
    FVector ActorRight = GetActorRightVector();
    
    // 다방향 Trace (더 많은 방향)
    TArray<FVector> TraceDirections;
    TraceDirections.Add(ActorForward);                                      // 앞
    TraceDirections.Add(-ActorUp);                                          // 아래
    TraceDirections.Add((ActorForward - ActorUp).GetSafeNormal());         // 앞+아래
    TraceDirections.Add((ActorForward + ActorUp).GetSafeNormal());         // 앞+위
    TraceDirections.Add((ActorForward - ActorRight).GetSafeNormal());      // 앞+왼쪽
    TraceDirections.Add((ActorForward + ActorRight).GetSafeNormal());      // 앞+오른쪽
    TraceDirections.Add((ActorForward - ActorUp - ActorRight).GetSafeNormal()); // 앞+아래+왼쪽
    TraceDirections.Add((ActorForward - ActorUp + ActorRight).GetSafeNormal()); // 앞+아래+오른쪽
    
    FHitResult BestHit;
    float ClosestDistance = WallTraceDistance * 2.0f;  // 거리 2배 증가
    bool bFoundWall = false;
    
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    for (const FVector& Direction : TraceDirections)
    {
        FVector TraceEnd = ActorLoc + Direction * WallTraceDistance;
        FHitResult Hit;
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, ActorLoc, TraceEnd, ECC_WorldStatic, Params))
        {
            float Distance = Hit.Distance;
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                BestHit = Hit;
                bFoundWall = true;
            }
        }
    }
    
    if (bFoundWall)
    {
        OutWallNormal = BestHit.Normal;
        OutHitLocation = BestHit.Location;
        return true;
    }
    
    return false;
}

void AWallCrawler::UpdateWallAlignment()
{
    FVector WallNormal, HitLocation;
    
    if (DetectWall(WallNormal, HitLocation))
    {
        CurrentWallNormal = WallNormal;
        WallHitLocation = HitLocation;
        bIsOnWall = true;
        
        // 벽에 수직으로 회전 (보간 속도 증가)
        FRotator TargetRotation = (-WallNormal).Rotation();
        
        // 급경사 대응: Normal 변화가 크면 빠르게 회전
        float NormalDifference = FVector::DotProduct(GetActorForwardVector(), -WallNormal);
        
        // 45도 근처 (DotProduct ~0.7)에서 특히 빠르게
        float InterpSpeed;
        if (FMath::Abs(NormalDifference) < 0.8f && FMath::Abs(NormalDifference) > 0.6f)
        {
            InterpSpeed = 20.0f;  // 45도 부근에서 매우 빠르게
        }
        else if (NormalDifference < 0.5f)
        {
            InterpSpeed = 15.0f;  // 급경사
        }
        else
        {
            InterpSpeed = 5.0f;   // 일반
        }
        
        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, 
            GetWorld()->GetDeltaSeconds(), InterpSpeed);
        SetActorRotation(NewRotation);
        
        // 벽에서 일정 거리 유지
        SetActorLocation(HitLocation + WallNormal * WallStickDistance);
    }
    else
    {
        bIsOnWall = false;
    }
}

void AWallCrawler::CrawlOnWall(FVector Direction, float Speed)
{
    if (!bIsOnWall) return;
    
    // 방향을 벽 표면에 투영하고 정규화
    FVector ProjectedDir = ProjectToWallSurface(Direction);
    
    // CRITICAL: 속도 일정하게 유지 (경사 무관)
    FVector Movement = ProjectedDir * Speed * GetWorld()->GetDeltaSeconds();
    
    // 새 위치로 이동
    FVector NewLocation = GetActorLocation() + Movement;
    SetActorLocation(NewLocation);
    
    // 이동 후 즉시 벽 재정렬 (경사 변화 대응)
    FVector WallNormal, HitLocation;
    if (DetectWall(WallNormal, HitLocation))
    {
        // 벽에 다시 붙이기
        SetActorLocation(HitLocation + WallNormal * WallStickDistance);
    }
}

FVector AWallCrawler::ProjectToWallSurface(FVector WorldDirection)
{
    if (!bIsOnWall) return WorldDirection.GetSafeNormal();
    
    // 벽 Normal에 수직인 성분만 남김
    FVector ProjectedDirection = WorldDirection - CurrentWallNormal * FVector::DotProduct(WorldDirection, CurrentWallNormal);
    
    // CRITICAL: 반드시 정규화 (길이 1.0 유지)
    FVector Result = ProjectedDirection.GetSafeNormal();
    
    // 안전 검사: 유효하지 않으면 Up 벡터 사용
    if (Result.IsNearlyZero())
    {
        FVector Right = FVector::CrossProduct(CurrentWallNormal, FVector::UpVector).GetSafeNormal();
        Result = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
    }
    
    return Result;
}

// ============================================
// CirclePatrol - 사용 안 함
// ============================================
/*
void AWallCrawler::CirclePatrol(float DeltaTime)
{
    PatrolAngle += PatrolAngularSpeed * DeltaTime;
    if (PatrolAngle >= 360.0f) PatrolAngle -= 360.0f;
    
    float AngleRad = FMath::DegreesToRadians(PatrolAngle);
    FVector Up = FVector::UpVector;
    FVector Right = FVector::CrossProduct(CurrentWallNormal, Up).GetSafeNormal();
    FVector ActualUp = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
    FVector CircleOffset = Right * FMath::Cos(AngleRad) * CircleRadius + ActualUp * FMath::Sin(AngleRad) * CircleRadius;
    FVector TargetLocation = PatrolCenter + CircleOffset;
    CrawlOnWall((TargetLocation - GetActorLocation()).GetSafeNormal(), CrawlSpeed);
}
*/

// ============================================
// Organic Patrol
// ============================================

void AWallCrawler::GeneratePatrolWaypoints()
{
    PatrolWaypoints.Empty();
    int32 NumWaypoints = FMath::RandRange(8, 12);
    
    FVector Up = FVector::UpVector;
    FVector Right = FVector::CrossProduct(CurrentWallNormal, Up).GetSafeNormal();
    FVector ActualUp = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
    
    for (int32 i = 0; i < NumWaypoints; i++)
    {
        float Angle = (float)i / NumWaypoints * 2.0f * PI + FMath::RandRange(-0.3f, 0.3f);
        float Radius = CircleRadius * FMath::RandRange(0.5f, 1.5f);
        
        FVector Offset = Right * FMath::Cos(Angle) * Radius + ActualUp * FMath::Sin(Angle) * Radius;
        FVector Waypoint = PatrolCenter + Offset;
        
        // 벽면 투영
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, 
            Waypoint - CurrentWallNormal * 100.0f, 
            Waypoint + CurrentWallNormal * 100.0f, 
            ECC_WorldStatic, Params))
        {
            Waypoint = Hit.Location + Hit.Normal * WallStickDistance;
        }
        
        PatrolWaypoints.Add(Waypoint);
    }
    
    CurrentWaypointIndex = 0;
    if (PatrolWaypoints.Num() > 0) CurrentTargetPoint = PatrolWaypoints[0];
}

void AWallCrawler::OrganicPatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;
    
    if (bIsPaused)
    {
        PauseTimer += DeltaTime;
        CurrentSpeed = 0.0f;
        
        if (PauseTimer >= NextPauseDuration)
        {
            bIsPaused = false;
            PauseTimer = 0.0f;
            TargetSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
        }
        return;
    }
    
    UpdateMovementSpeed(DeltaTime);
    
    float DistanceToTarget = (CurrentTargetPoint - GetActorLocation()).Size();
    
    if (DistanceToTarget < WaypointReachThreshold)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        CurrentTargetPoint = PatrolWaypoints[CurrentWaypointIndex];
        
        if (FMath::FRand() < PauseChance)
        {
            bIsPaused = true;
            NextPauseDuration = FMath::RandRange(MinPauseDuration, MaxPauseDuration);
        }
        
        TargetSpeed = FMath::RandRange(MinSpeed, MaxSpeed);
        
        if (CurrentWaypointIndex == 0 && FMath::FRand() < 0.2f)
        {
            GeneratePatrolWaypoints();
        }
    }
    
    CrawlOnWall((CurrentTargetPoint - GetActorLocation()).GetSafeNormal(), CurrentSpeed);
}

void AWallCrawler::UpdateMovementSpeed(float DeltaTime)
{
    if (FMath::Abs(CurrentSpeed - TargetSpeed) > 1.0f)
    {
        CurrentSpeed += (CurrentSpeed < TargetSpeed ? 1 : -1) * SpeedChangeRate * DeltaTime;
        CurrentSpeed = FMath::Clamp(CurrentSpeed, FMath::Min(CurrentSpeed, TargetSpeed), FMath::Max(CurrentSpeed, TargetSpeed));
    }
}

// ============================================
// Attack
// ============================================

void AWallCrawler::PursuePlayer(float DeltaTime)
{
    if (!TargetPlayer) return;
    
    // 플레이어보다 약간 앞쪽 목표 (빠른 달라붙기)
    FVector PlayerVelocity = TargetPlayer->GetVelocity();
    FVector PredictedLocation = TargetPlayer->GetActorLocation() + PlayerVelocity * 0.3f;  // 0.3초 예측
    
    FVector ToPlayer = PredictedLocation - GetActorLocation();
    CrawlOnWall(ProjectToWallSurface(ToPlayer.GetSafeNormal()), PursuitSpeed);
}

void AWallCrawler::AttachToPlayer(ADownfallCharacter* Player)
{
    if (!Player || bAttachedToPlayer) return;
    
    bAttachedToPlayer = true;
    TargetPlayer = Player;
    AccumulatedShake = 0.0f;
    DetectionGauge = 0.0f;
    PotentialTarget = nullptr;
    
    // 몬스터 숨기기 (디버프 모드)
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(true);  // Tick은 유지 (Stamina 흡수용)
    
    // 위치는 플레이어 근처로만 (보이지 않음)
    SetActorLocation(Player->GetActorLocation() + FVector(0, 0, 100));
    
    UE_LOG(LogMonster, Warning, TEXT("%s ATTACHED to player (hidden debuff mode)!"), *GetName());
}

void AWallCrawler::DetachFromPlayer()
{
    if (!bAttachedToPlayer) return;
    
    bAttachedToPlayer = false;
    AccumulatedShake = 0.0f;
    
    // 몬스터 다시 나타내기
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    if (TargetPlayer)
    {
        FVector PlayerLoc = TargetPlayer->GetActorLocation();
        
        // 플레이어 주변 랜덤 방향
        FVector RandomDir = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-0.5f, 0.5f)
        ).GetSafeNormal();
        
        // 플레이어에서 100-150cm 거리
        FVector TestLocation = PlayerLoc + RandomDir * FMath::RandRange(100.0f, 150.0f);
        
        // 그 위치에서 가장 가까운 벽 찾기
        bool bFoundWall = false;
        
        // 여러 방향으로 벽 찾기
        TArray<FVector> SearchDirections;
        SearchDirections.Add(RandomDir);
        SearchDirections.Add(-RandomDir);
        SearchDirections.Add(FVector::UpVector);
        SearchDirections.Add(-FVector::UpVector);
        SearchDirections.Add(FVector::RightVector);
        SearchDirections.Add(-FVector::RightVector);
        
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(TargetPlayer);
        
        float BestDistance = 999999.0f;
        FVector BestWallLocation;
        FVector BestWallNormal;
        
        for (const FVector& Dir : SearchDirections)
        {
            FHitResult Hit;
            FVector TraceStart = TestLocation;
            FVector TraceEnd = TestLocation + Dir * 300.0f;  // 3m 범위
            
            if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
            {
                float Distance = Hit.Distance;
                if (Distance < BestDistance)
                {
                    BestDistance = Distance;
                    BestWallLocation = Hit.Location;
                    BestWallNormal = Hit.Normal;
                    bFoundWall = true;
                }
            }
        }
        
        if (bFoundWall)
        {
            // 벽에서 플레이어 쪽으로 WallStickDistance만큼 떨어진 위치에 스폰
            // Normal이 플레이어를 향하도록 확인
            FVector ToPlayer = PlayerLoc - BestWallLocation;
            float DotProduct = FVector::DotProduct(BestWallNormal, ToPlayer.GetSafeNormal());
            
            // Normal이 벽 안쪽을 향하면 반대로
            if (DotProduct < 0)
            {
                BestWallNormal = -BestWallNormal;
            }
            
            FVector SpawnLocation = BestWallLocation + BestWallNormal * WallStickDistance;
            SetActorLocation(SpawnLocation);
            
            // 벽을 향하도록 회전
            FRotator TargetRotation = (-BestWallNormal).Rotation();
            SetActorRotation(TargetRotation);
            
            // 멤버 변수 업데이트
            CurrentWallNormal = BestWallNormal;
            WallHitLocation = BestWallLocation;
            bIsOnWall = true;
            
            UE_LOG(LogMonster, Warning, TEXT("%s DETACHED! Respawning on wall at %.1fcm from player"), 
                *GetName(), FVector::Dist(SpawnLocation, PlayerLoc));
        }
        else
        {
            // 벽을 못 찾았으면 플레이어 근처에 그냥 스폰
            SetActorLocation(TestLocation);
            bIsOnWall = false;
            
            UE_LOG(LogMonster, Warning, TEXT("%s DETACHED! No wall found, spawning at %.1fcm from player"), 
                *GetName(), FVector::Dist(TestLocation, PlayerLoc));
        }
    }
}

void AWallCrawler::DrainStamina(float DeltaTime)
{
    if (!TargetPlayer || !bAttachedToPlayer) return;
    
    float DrainAmount = StaminaDrainRate * DeltaTime;
    
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
    if (!TargetPlayer || !bAttachedToPlayer) return;
    
    APlayerController* PC = Cast<APlayerController>(TargetPlayer->GetController());
    if (!PC) return;
    
    // 카메라 회전 변화량으로 Shake 감지
    FRotator CurrentRotation = PC->GetControlRotation();
    
    if (LastMousePosition.IsNearlyZero())
    {
        // 첫 프레임: 현재 회전을 저장 (X=Pitch, Y=Yaw로 변환)
        LastMousePosition = FVector2D(CurrentRotation.Pitch, CurrentRotation.Yaw);
        return;
    }
    
    // 이전 회전과 현재 회전의 차이 계산
    FVector2D CurrentRotation2D(CurrentRotation.Pitch, CurrentRotation.Yaw);
    FVector2D RotationDelta = CurrentRotation2D - LastMousePosition;
    
    // Yaw의 360도 wrap 처리
    if (RotationDelta.Y > 180.0f) RotationDelta.Y -= 360.0f;
    if (RotationDelta.Y < -180.0f) RotationDelta.Y += 360.0f;
    
    // 회전 변화량을 Shake로 누적 (좌우 흔들기 = Yaw 변화)
    float RotationMovement = FMath::Abs(RotationDelta.Y);  // Yaw 변화만 사용
    AccumulatedShake += RotationMovement * 5.0f;  // 배율 적용
    
    LastMousePosition = CurrentRotation2D;
    
    if (AccumulatedShake >= ShakeThreshold)
    {
        UE_LOG(LogMonster, Warning, TEXT("%s shaken off! (%.1f accumulated)"), *GetName(), AccumulatedShake);
        DetachFromPlayer();
    }
    
#if !UE_BUILD_SHIPPING
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, 
            AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow,
            FString::Printf(TEXT("Shake: %.0f / %.0f (Yaw: %.1f)"), AccumulatedShake, ShakeThreshold, RotationMovement));
    }
#endif
}

// ============================================
// Detection Gauge System (감지 게이지)
// ============================================

void AWallCrawler::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    ADownfallCharacter* Player = Cast<ADownfallCharacter>(Actor);
    if (!Player) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // 플레이어 움직임 감지 - PotentialTarget 설정
        PotentialTarget = Player;
        UE_LOG(LogMonster, Verbose, TEXT("%s heard noise from player"), *GetName());
    }
    else
    {
        // 플레이어 소리 사라짐
        if (PotentialTarget == Player)
        {
            PotentialTarget = nullptr;
        }
    }
}

void AWallCrawler::UpdateDetectionGauge(float DeltaTime)
{
    if (PotentialTarget)
    {
        // 소음 감지 중 - 게이지 증가
        DetectionGauge += DetectionGainRate * DeltaTime;
        DetectionGauge = FMath::Min(DetectionGauge, DetectionGaugeMax);
        
        // 게이지 가득 참 → Target 확정!
        if (DetectionGauge >= DetectionGaugeMax && !TargetPlayer)
        {
            TargetPlayer = Cast<ADownfallCharacter>(PotentialTarget);
            UE_LOG(LogMonster, Warning, TEXT("%s DETECTED player (gauge full: %.1f)"), *GetName(), DetectionGauge);
        }
    }
    else
    {
        // 소음 없음 - 게이지 감소
        if (DetectionGauge > 0.0f)
        {
            DetectionGauge -= DetectionDecayRate * DeltaTime;
            DetectionGauge = FMath::Max(DetectionGauge, 0.0f);
            
            // 게이지 0 → Target 해제
            if (DetectionGauge <= 0.0f && TargetPlayer)
            {
                TargetPlayer = nullptr;
                UE_LOG(LogMonster, Warning, TEXT("%s LOST player (gauge empty)"), *GetName());
            }
        }
    }
    
    // 거리 체크 (기존)
    if (TargetPlayer)
    {
        float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
        if (Distance > 1500.0f)
        {
            TargetPlayer = nullptr;
            PotentialTarget = nullptr;
            DetectionGauge = 0.0f;
            UE_LOG(LogMonster, Warning, TEXT("%s lost player (too far: %.1f)"), *GetName(), Distance);
        }
    }
}