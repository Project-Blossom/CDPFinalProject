#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Perception/AISenseConfig_Hearing.h"

AWallCrawler::AWallCrawler()
{
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
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
        HearingConfig->SetMaxAge(2.0f);
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
            else if (Distance <= AttachRange && TargetPlayer->GetCharacterMovement()->MovementMode == MOVE_Flying)
            {
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
    
    // TargetPlayer 상태
    if (TargetPlayer)
    {
        DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 70), 
            FString::Printf(TEXT("TARGET: %.0fcm"), FVector::Dist(ActorLoc, TargetPlayer->GetActorLocation())), 
            nullptr, FColor::Red, 0.0f, true);
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
    
    FVector ToPlayer = TargetPlayer->GetActorLocation() - GetActorLocation();
    CrawlOnWall(ProjectToWallSurface(ToPlayer.GetSafeNormal()), PursuitSpeed);
}

void AWallCrawler::AttachToPlayer(ADownfallCharacter* Player)
{
    if (!Player || bAttachedToPlayer) return;
    
    bAttachedToPlayer = true;
    TargetPlayer = Player;
    AccumulatedShake = 0.0f;
    
    AttachToActor(Player, FAttachmentTransformRules::KeepWorldTransform);
    SetActorRelativeLocation(FVector(0, 0, -30.0f));
    
    UE_LOG(LogMonster, Warning, TEXT("%s ATTACHED to player!"), *GetName());
}

void AWallCrawler::DetachFromPlayer()
{
    if (!bAttachedToPlayer) return;
    
    bAttachedToPlayer = false;
    AccumulatedShake = 0.0f;
    
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorLocation(GetActorLocation() - GetActorForwardVector() * 100.0f);
    
    UE_LOG(LogMonster, Warning, TEXT("%s DETACHED!"), *GetName());
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
    
    float MouseX, MouseY;
    PC->GetMousePosition(MouseX, MouseY);
    FVector2D CurrentPos(MouseX, MouseY);
    
    if (LastMousePosition.IsNearlyZero())
    {
        LastMousePosition = CurrentPos;
        return;
    }
    
    AccumulatedShake += (CurrentPos - LastMousePosition).Size();
    LastMousePosition = CurrentPos;
    
    if (AccumulatedShake >= ShakeThreshold)
    {
        DetachFromPlayer();
    }
    
#if !UE_BUILD_SHIPPING
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, 
            AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow,
            FString::Printf(TEXT("Shake: %.0f / %.0f"), AccumulatedShake, ShakeThreshold));
    }
#endif
}