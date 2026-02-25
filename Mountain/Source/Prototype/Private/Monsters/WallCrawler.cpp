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
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AWallCrawler::BeginPlay()
{
    Super::BeginPlay();
    
    // Hearing만 사용 (방향 무관)
    if (PerceptionComponent)
    {
        UAISenseConfig_Hearing* HearingConfig = NewObject<UAISenseConfig_Hearing>(this);
        HearingConfig->HearingRange = 1000.0f;
        HearingConfig->SetMaxAge(0.2f);
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        
        PerceptionComponent->ConfigureSense(*HearingConfig);
        PerceptionComponent->SetDominantSense(HearingConfig->GetSenseImplementation());
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
            
            if (Distance > 1500.0f)
            {
                TargetPlayer = nullptr;
            }
            else if (Distance <= AttachRange)
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
    if (!bAttachedToPlayer)
    {
        FVector ActorLoc = GetActorLocation();
        
        // AI 감지 범위
        DrawDebugSphere(GetWorld(), ActorLoc, 1000.0f, 16, FColor::Yellow, false, 0.1f, 0, 1.0f);
        
        // 상태 텍스트
        DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 50), 
            bIsOnWall ? TEXT("ON WALL") : TEXT("NO WALL"),
            nullptr, bIsOnWall ? FColor::Green : FColor::Red, 0.0f, true);
        
        // 감지 게이지
        if (DetectionGauge > 0.0f || PotentialTarget)
        {
            FColor GaugeColor = DetectionGauge >= DetectionGaugeMax ? FColor::Red : 
                               DetectionGauge > DetectionGaugeMax * 0.5f ? FColor::Orange : FColor::Yellow;
            
            DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 90), 
                FString::Printf(TEXT("Gauge: %.0f/%.0f"), DetectionGauge, DetectionGaugeMax), 
                nullptr, GaugeColor, 0.0f, true);
        }
        
        // Target 상태
        if (TargetPlayer)
        {
            DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 70), 
                FString::Printf(TEXT("TARGET: %.0fcm"), FVector::Dist(ActorLoc, TargetPlayer->GetActorLocation())), 
                nullptr, FColor::Red, 0.0f, true);
        }
    }
    else
    {
        // Shake 게이지
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, 
                AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow,
                FString::Printf(TEXT("WallCrawler Attached! Shake: %.0f / %.0f"), AccumulatedShake, ShakeThreshold));
        }
    }
#endif
}

// Wall Detection
bool AWallCrawler::DetectWall(FVector& OutWallNormal, FVector& OutHitLocation)
{
    FVector ActorLoc = GetActorLocation();
    FVector ActorForward = GetActorForwardVector();
    FVector ActorUp = GetActorUpVector();
    FVector ActorRight = GetActorRightVector();
    
    TArray<FVector> TraceDirections;
    TraceDirections.Add(ActorForward);
    TraceDirections.Add(-ActorUp);
    TraceDirections.Add((ActorForward - ActorUp).GetSafeNormal());
    TraceDirections.Add((ActorForward + ActorUp).GetSafeNormal());
    TraceDirections.Add((ActorForward - ActorRight).GetSafeNormal());
    TraceDirections.Add((ActorForward + ActorRight).GetSafeNormal());
    TraceDirections.Add((ActorForward - ActorUp - ActorRight).GetSafeNormal());
    TraceDirections.Add((ActorForward - ActorUp + ActorRight).GetSafeNormal());
    
    FHitResult BestHit;
    float ClosestDistance = WallTraceDistance * 2.0f;
    bool bFoundWall = false;
    
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    for (const FVector& Direction : TraceDirections)
    {
        FVector TraceEnd = ActorLoc + Direction * WallTraceDistance;
        FHitResult Hit;
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, ActorLoc, TraceEnd, ECC_WorldStatic, Params))
        {
            if (Hit.Distance < ClosestDistance)
            {
                ClosestDistance = Hit.Distance;
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
        
        // 경사 대응 회전 속도
        float NormalDifference = FVector::DotProduct(GetActorForwardVector(), -WallNormal);
        float InterpSpeed = (FMath::Abs(NormalDifference) < 0.8f && FMath::Abs(NormalDifference) > 0.6f) ? 20.0f :
                           (NormalDifference < 0.5f) ? 15.0f : 5.0f;
        
        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), (-WallNormal).Rotation(), 
            GetWorld()->GetDeltaSeconds(), InterpSpeed);
        SetActorRotation(NewRotation);
        SetActorLocation(HitLocation + WallNormal * WallStickDistance);
    }
    else
    {
        bIsOnWall = false;
    }
}

// Wall Movement
void AWallCrawler::CrawlOnWall(FVector Direction, float Speed)
{
    if (!bIsOnWall) return;
    
    FVector ProjectedDir = ProjectToWallSurface(Direction);
    FVector Movement = ProjectedDir * Speed * GetWorld()->GetDeltaSeconds();
    FVector NewLocation = GetActorLocation() + Movement;
    SetActorLocation(NewLocation);
    
    // 재정렬
    FVector WallNormal, HitLocation;
    if (DetectWall(WallNormal, HitLocation))
    {
        SetActorLocation(HitLocation + WallNormal * WallStickDistance);
    }
}

FVector AWallCrawler::ProjectToWallSurface(FVector WorldDirection)
{
    if (!bIsOnWall) return WorldDirection.GetSafeNormal();
    
    FVector ProjectedDirection = WorldDirection - CurrentWallNormal * FVector::DotProduct(WorldDirection, CurrentWallNormal);
    FVector Result = ProjectedDirection.GetSafeNormal();
    
    if (Result.IsNearlyZero())
    {
        FVector Right = FVector::CrossProduct(CurrentWallNormal, FVector::UpVector).GetSafeNormal();
        Result = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
    }
    
    return Result;
}

// Organic Patrol
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

// Attack
void AWallCrawler::PursuePlayer(float DeltaTime)
{
    if (!TargetPlayer) return;
    
    FVector PlayerVelocity = TargetPlayer->GetVelocity();
    FVector PredictedLocation = TargetPlayer->GetActorLocation() + PlayerVelocity * 0.3f;
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
    
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(true);
    SetActorLocation(Player->GetActorLocation() + FVector(0, 0, 100));
    
    UE_LOG(LogMonster, Warning, TEXT("%s ATTACHED (debuff mode)"), *GetName());
}

void AWallCrawler::DetachFromPlayer()
{
    if (!bAttachedToPlayer) return;
    
    bAttachedToPlayer = false;
    AccumulatedShake = 0.0f;
    
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    if (TargetPlayer)
    {
        FVector PlayerLoc = TargetPlayer->GetActorLocation();
        FVector RandomDir = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 
            FMath::RandRange(-0.5f, 0.5f)).GetSafeNormal();
        FVector TestLocation = PlayerLoc + RandomDir * FMath::RandRange(100.0f, 150.0f);
        
        // 가까운 벽 찾기
        TArray<FVector> SearchDirections = {RandomDir, -RandomDir, FVector::UpVector, 
            -FVector::UpVector, FVector::RightVector, -FVector::RightVector};
        
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(TargetPlayer);
        
        float BestDistance = 999999.0f;
        FVector BestWallLocation, BestWallNormal;
        bool bFoundWall = false;
        
        for (const FVector& Dir : SearchDirections)
        {
            FHitResult Hit;
            if (GetWorld()->LineTraceSingleByChannel(Hit, TestLocation, TestLocation + Dir * 300.0f, ECC_WorldStatic, Params))
            {
                if (Hit.Distance < BestDistance)
                {
                    BestDistance = Hit.Distance;
                    BestWallLocation = Hit.Location;
                    BestWallNormal = Hit.Normal;
                    bFoundWall = true;
                }
            }
        }
        
        if (bFoundWall)
        {
            // Normal 방향 확인
            if (FVector::DotProduct(BestWallNormal, (PlayerLoc - BestWallLocation).GetSafeNormal()) < 0)
            {
                BestWallNormal = -BestWallNormal;
            }
            
            FVector SpawnLocation = BestWallLocation + BestWallNormal * WallStickDistance;
            SetActorLocation(SpawnLocation);
            SetActorRotation((-BestWallNormal).Rotation());
            
            CurrentWallNormal = BestWallNormal;
            WallHitLocation = BestWallLocation;
            bIsOnWall = true;
        }
        else
        {
            SetActorLocation(TestLocation);
            bIsOnWall = false;
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
    
    FRotator CurrentRotation = PC->GetControlRotation();
    
    if (LastMousePosition.IsNearlyZero())
    {
        LastMousePosition = FVector2D(CurrentRotation.Pitch, CurrentRotation.Yaw);
        return;
    }
    
    FVector2D CurrentRotation2D(CurrentRotation.Pitch, CurrentRotation.Yaw);
    FVector2D RotationDelta = CurrentRotation2D - LastMousePosition;
    
    // Yaw wrap 처리
    if (RotationDelta.Y > 180.0f) RotationDelta.Y -= 360.0f;
    if (RotationDelta.Y < -180.0f) RotationDelta.Y += 360.0f;
    
    float RotationMovement = FMath::Abs(RotationDelta.Y);
    AccumulatedShake += RotationMovement * 5.0f;
    LastMousePosition = CurrentRotation2D;
    
    if (AccumulatedShake >= ShakeThreshold)
    {
        UE_LOG(LogMonster, Warning, TEXT("%s shaken off!"), *GetName());
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

// Detection Gauge
void AWallCrawler::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    ADownfallCharacter* Player = Cast<ADownfallCharacter>(Actor);
    if (!Player) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        PotentialTarget = Player;
    }
    else
    {
        if (PotentialTarget == Player) PotentialTarget = nullptr;
    }
}

void AWallCrawler::UpdateDetectionGauge(float DeltaTime)
{
    if (PotentialTarget)
    {
        DetectionGauge += DetectionGainRate * DeltaTime;
        DetectionGauge = FMath::Min(DetectionGauge, DetectionGaugeMax);
        
        if (DetectionGauge >= DetectionGaugeMax && !TargetPlayer)
        {
            TargetPlayer = Cast<ADownfallCharacter>(PotentialTarget);
            UE_LOG(LogMonster, Warning, TEXT("%s DETECTED player"), *GetName());
        }
    }
    else
    {
        if (DetectionGauge > 0.0f)
        {
            DetectionGauge -= DetectionDecayRate * DeltaTime;
            DetectionGauge = FMath::Max(DetectionGauge, 0.0f);
            
            if (DetectionGauge <= 0.0f && TargetPlayer)
            {
                TargetPlayer = nullptr;
                UE_LOG(LogMonster, Warning, TEXT("%s LOST player"), *GetName());
            }
        }
    }
    
    if (TargetPlayer)
    {
        float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
        if (Distance > 1500.0f)
        {
            TargetPlayer = nullptr;
            PotentialTarget = nullptr;
            DetectionGauge = 0.0f;
        }
    }
}