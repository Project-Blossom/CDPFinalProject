#include "Monsters/WallCrawler.h"
#include "DownfallCharacter.h"
#include "Monsters/FlyingPlatform.h"  // ← 추가!
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TimerManager.h"  // ← 추가 (FTimerHandle 사용)

AWallCrawler::AWallCrawler()
{
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AWallCrawler::BeginPlay()
{
    Super::BeginPlay();
    
    // CRITICAL: 스폰 시간 기록
    SpawnTime = GetWorld()->GetTimeSeconds();
    
    // CRITICAL: TargetPlayer 명시적 초기화
    TargetPlayer = nullptr;
    DetectionGauge = 0.0f;
    PotentialTarget = nullptr;
    bAttachedToPlayer = false;
    bIsStunned = false;
    StunTimer = 0.0f;
    
    // Blackboard 초기화 (BT 사용 시)
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
        
        // LastAttachTime을 과거로 설정 (즉시 attach 가능)
        Blackboard->SetValueAsFloat("LastAttachTime", -100.0f);
        
        // bCanAttach을 true로 설정
        Blackboard->SetValueAsBool("bCanAttach", true);
        
        // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s Blackboard initialized (bCanAttach: true)"), *GetName());
    }
    
    // [DISABLED FOR DEMO] UE_LOG(LogMonster, Warning, TEXT("%s BeginPlay: TargetPlayer = NULL, DetectionGauge = 0, SpawnTime = %.1f"), *GetName(), SpawnTime);
    
    // CRITICAL: MonsterBase의 OnPerceptionUpdated 바인딩 해제 후 WallCrawler 것으로 재바인딩
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AWallCrawler::OnPerceptionUpdated);
        
        // Hearing만 사용 (방향 무관)
        UAISenseConfig_Hearing* HearingConfig = NewObject<UAISenseConfig_Hearing>(this);
        HearingConfig->HearingRange = 1000.0f;
        HearingConfig->SetMaxAge(0.2f);
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        
        PerceptionComponent->ConfigureSense(*HearingConfig);
        PerceptionComponent->SetDominantSense(HearingConfig->GetSenseImplementation());
        
        UE_LOG(LogMonster, Warning, TEXT("%s: Perception rebound to WallCrawler version"), *GetName());
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
    
    UE_LOG(LogMonster, Log, TEXT("%s (Wall Crawler) ready to crawl"), *GetName());
}

void AWallCrawler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Behavior Tree가 실행 중이면 기존 로직 스킵
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->BrainComponent && AIController->BrainComponent->IsRunning())
    {
        // BT가 실행 중일 때는 기본 AI 로직 스킵
        // Stun 처리와 DetectionGauge만 업데이트
        if (bIsStunned)
        {
            StunTimer -= DeltaTime;
            if (StunTimer <= 0.0f)
            {
                bIsStunned = false;
                UE_LOG(LogMonster, Log, TEXT("%s recovered from stun"), *GetName());
            }
            
            if (!bIsOnWall)
            {
                ApplyGravity(DeltaTime);
            }
        }
        
        UpdateWallAlignment();
        UpdateDetectionGauge(DeltaTime);
        
        if (!bIsOnWall)
        {
            ApplyGravity(DeltaTime);
        }
        
        if (bAttachedToPlayer)
        {
            DrainStamina(DeltaTime);
            UpdateShakeDetection(DeltaTime);
        }
        
        return;
    }
    
    // 이하 기존 로직 (BT 없을 때만 실행)
    
    // Stun 처리
    if (bIsStunned)
    {
        StunTimer -= DeltaTime;
        if (StunTimer <= 0.0f)
        {
            bIsStunned = false;
            UE_LOG(LogMonster, Log, TEXT("%s recovered from stun"), *GetName());
        }
        
        // Stun 중에는 중력 적용 (벽에 없으면)
        if (!bIsOnWall)
        {
            ApplyGravity(DeltaTime);
        }
        
        // Stun 중에는 다른 행동 안 함
        return;
    }

    UpdateWallAlignment();
    UpdateDetectionGauge(DeltaTime);
    
    // 벽에 없으면 중력 적용 (새로 추가)
    if (!bIsOnWall)
    {
        ApplyGravity(DeltaTime);
    }

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
                UE_LOG(LogMonster, Warning, TEXT("%s: Player too far (%.1fcm) - TargetPlayer set to NULL"), *GetName(), Distance);
            }
            else if (Distance <= AttachRange)
            {
                UE_LOG(LogMonster, Error, TEXT("%s: ATTACHING! Distance: %.1fcm <= AttachRange: %.1fcm"), 
                    *GetName(), Distance, AttachRange);
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

// [DISABLED FOR DEMO] Debug visualization: AI range, status text, gauge, target info
#if 0
    if (!bAttachedToPlayer)
    {
        FVector ActorLoc = GetActorLocation();
        DrawDebugSphere(GetWorld(), ActorLoc, 1000.0f, 16, FColor::Yellow, false, 0.1f, 0, 1.0f);
        FString StatusText = bIsStunned ? TEXT("STUNNED") : bIsOnWall ? TEXT("ON WALL") : TEXT("NO WALL");
        FColor StatusColor = bIsStunned ? FColor::Purple : bIsOnWall ? FColor::Green : FColor::Red;
        DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 50), StatusText, nullptr, StatusColor, 0.0f, true);
        if (bIsStunned)
        {
            DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 70), FString::Printf(TEXT("Stun: %.1fs"), StunTimer), nullptr, FColor::Purple, 0.0f, true);
        }
        if (DetectionGauge > 0.0f || PotentialTarget)
        {
            FColor GaugeColor = DetectionGauge >= DetectionGaugeMax ? FColor::Red : DetectionGauge > DetectionGaugeMax * 0.5f ? FColor::Orange : FColor::Yellow;
            DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 90), FString::Printf(TEXT("Gauge: %.0f/%.0f"), DetectionGauge, DetectionGaugeMax), nullptr, GaugeColor, 0.0f, true);
        }
        if (TargetPlayer)
        {
            DrawDebugString(GetWorld(), ActorLoc + FVector(0, 0, 110), FString::Printf(TEXT("TARGET: %.0fcm"), FVector::Dist(ActorLoc, TargetPlayer->GetActorLocation())), nullptr, FColor::Red, 0.0f, true);
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow, FString::Printf(TEXT("WallCrawler Attached! Shake: %.0f / %.0f"), AccumulatedShake, ShakeThreshold));
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
        // 부드러운 Normal 변화 (급격한 변화 방지)
        if (!CurrentWallNormal.IsNearlyZero())
        {
            // 이전 Normal과 너무 다르면 부드럽게 보간
            float DotProduct = FVector::DotProduct(CurrentWallNormal, WallNormal);
            if (DotProduct > 0.5f)  // 60도 이내
            {
                WallNormal = FMath::VInterpTo(CurrentWallNormal, WallNormal, GetWorld()->GetDeltaSeconds(), 3.0f);
            }
        }
        
        CurrentWallNormal = WallNormal;
        WallHitLocation = HitLocation;
        bIsOnWall = true;
        
        // 회전 (배회 중일 때는 더 느리게)
        float RotationSpeed = TargetPlayer ? 10.0f : 3.0f;  // 배회: 3, 추격: 10
        
        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), (-WallNormal).Rotation(), 
            GetWorld()->GetDeltaSeconds(), RotationSpeed);
        SetActorRotation(NewRotation);
        
        // 위치 업데이트 (더 멀리)
        SetActorLocation(HitLocation + WallNormal * (WallStickDistance + 5.0f));
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
    int32 NumWaypoints = FMath::RandRange(6, 8);
    
    FVector Up = FVector::UpVector;
    FVector Right = FVector::CrossProduct(CurrentWallNormal, Up).GetSafeNormal();
    FVector ActualUp = FVector::CrossProduct(Right, CurrentWallNormal).GetSafeNormal();
    
    for (int32 i = 0; i < NumWaypoints; i++)
    {
        float Angle = (float)i / NumWaypoints * 2.0f * PI + FMath::RandRange(-0.2f, 0.2f);
        float Radius = CircleRadius * FMath::RandRange(0.7f, 1.3f);
        
        FVector Offset = Right * FMath::Cos(Angle) * Radius + ActualUp * FMath::Sin(Angle) * Radius;
        FVector TargetPoint = PatrolCenter + Offset;
        
        // 벽면 투영
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        FVector TraceStart = TargetPoint - CurrentWallNormal * 200.0f;
        FVector TraceEnd = TargetPoint + CurrentWallNormal * 200.0f;
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
        {
            FVector Waypoint = Hit.Location + Hit.Normal * (WallStickDistance + 10.0f);
            PatrolWaypoints.Add(Waypoint);
        }
        else
        {
            PatrolWaypoints.Add(PatrolCenter + Offset.GetSafeNormal() * (Radius * 0.5f));
        }
    }
    
    CurrentWaypointIndex = 0;
    if (PatrolWaypoints.Num() > 0) CurrentTargetPoint = PatrolWaypoints[0];
    
    UE_LOG(LogMonster, Log, TEXT("%s generated %d waypoints"), *GetName(), PatrolWaypoints.Num());
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
    
    FVector ToTarget = CurrentTargetPoint - GetActorLocation();
    float DistanceToTarget = ToTarget.Size();
    
    float AdjustedThreshold = WaypointReachThreshold * 2.0f;
    
    if (DistanceToTarget < AdjustedThreshold)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        CurrentTargetPoint = PatrolWaypoints[CurrentWaypointIndex];
        
        if (FMath::FRand() < PauseChance * 0.5f)
        {
            bIsPaused = true;
            NextPauseDuration = FMath::RandRange(MinPauseDuration, MaxPauseDuration);
        }
        
        TargetSpeed = FMath::RandRange(MinSpeed * 0.7f, MaxSpeed * 0.7f);
        
        if (CurrentWaypointIndex == 0 && FMath::FRand() < 0.1f)
        {
            GeneratePatrolWaypoints();
        }
    }
    
    FVector CurrentDirection = ToTarget.GetSafeNormal();
    CrawlOnWall(CurrentDirection, CurrentSpeed);
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
    if (!Player || bAttachedToPlayer || bIsStunned)
        return;
    
    // CRITICAL: 스폰 직후 Attach 방지
    float TimeSinceSpawn = GetWorld()->GetTimeSeconds() - SpawnTime;
    if (TimeSinceSpawn < MinTimeBeforeAttach)
    {
        UE_LOG(LogMonster, Error, TEXT("%s ATTACH BLOCKED! Too soon after spawn (%.1fs < %.1fs)"), 
            *GetName(), TimeSinceSpawn, MinTimeBeforeAttach);
        return;
    }
    
    bAttachedToPlayer = true;
    TargetPlayer = Player;
    AccumulatedShake = 0.0f;
    DetectionGauge = 0.0f;
    PotentialTarget = nullptr;
    
    // CRITICAL: Shake 감지 초기화
    LastMousePosition = FVector2D::ZeroVector;  // 리셋
    
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(true);
    SetActorLocation(Player->GetActorLocation() + FVector(0, 0, 100));
    
    // Attach Desaturation VFX 표시
    Player->ShowAttachDesaturation(0.8f);
    
    UE_LOG(LogMonster, Warning, TEXT("%s ATTACHED (debuff mode) - Time since spawn: %.1fs, Shake detection RESET"), *GetName(), TimeSinceSpawn);
}

void AWallCrawler::DetachFromPlayer()
{
    if (!bAttachedToPlayer) return;
    
    bAttachedToPlayer = false;
    AccumulatedShake = 0.0f;
    LastMousePosition = FVector2D::ZeroVector;  // CRITICAL: 리셋
    
    // Stun 상태 시작
    bIsStunned = true;
    StunTimer = StunDuration;
    
    // Attach Desaturation VFX 숨김
    if (TargetPlayer)
    {
        TargetPlayer->HideAttachDesaturation();
    }
    
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    if (TargetPlayer)
    {
        FVector PlayerLoc = TargetPlayer->GetActorLocation();
        FVector RandomDir = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 
            FMath::RandRange(-0.5f, 0.5f)).GetSafeNormal();
        
        float SpawnDistance = FMath::RandRange(100.0f, 300.0f);  // 100~300cm 거리
        FVector TestLocation = PlayerLoc + RandomDir * SpawnDistance;
        
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
            
            UE_LOG(LogMonster, Warning, TEXT("%s DETACHED to wall, STUNNED for %.1fs at distance %.1fcm"), 
                *GetName(), StunDuration, SpawnDistance);
        }
        else
        {
            // 벽 못 찾음 - 허공에 스폰
            SetActorLocation(TestLocation);
            bIsOnWall = false;
            
            UE_LOG(LogMonster, Warning, TEXT("%s DETACHED to air (NoWall), STUNNED for %.1fs, will fall"), 
                *GetName(), StunDuration);
        }
    }
}

// 중력 적용 (새로 추가)
void AWallCrawler::ApplyGravity(float DeltaTime)
{
    FVector CurrentLocation = GetActorLocation();
    FVector Gravity = FVector(0, 0, -980.0f) * GravityScale; // 중력 가속도
    FVector NewLocation = CurrentLocation + Gravity * DeltaTime;
    
    // 지면 체크
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(Hit, CurrentLocation, NewLocation, ECC_WorldStatic, Params))
    {
        // 지면 도달
        SetActorLocation(Hit.Location + Hit.Normal * WallStickDistance);
        
        // 벽으로 간주
        CurrentWallNormal = Hit.Normal;
        WallHitLocation = Hit.Location;
        bIsOnWall = true;
        
        UE_LOG(LogMonster, Log, TEXT("%s hit ground, now on wall"), *GetName());
    }
    else
    {
        // 계속 낙하
        SetActorLocation(NewLocation);
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
    FVector2D CurrentRotation2D(CurrentRotation.Pitch, CurrentRotation.Yaw);
    
    // 첫 프레임: 현재 회전 저장
    if (LastMousePosition.IsZero())
    {
        LastMousePosition = CurrentRotation2D;
        UE_LOG(LogMonster, Log, TEXT("%s Shake detection initialized: Pitch=%.1f, Yaw=%.1f"), 
            *GetName(), CurrentRotation.Pitch, CurrentRotation.Yaw);
        return;
    }
    
    // 회전 변화량 계산
    FVector2D RotationDelta = CurrentRotation2D - LastMousePosition;
    
    // Yaw wrap 처리 (360도 경계)
    if (RotationDelta.Y > 180.0f) RotationDelta.Y -= 360.0f;
    if (RotationDelta.Y < -180.0f) RotationDelta.Y += 360.0f;
    
    // 절대값 합산 (양방향 흔들기 모두 카운트)
    float FrameShake = (FMath::Abs(RotationDelta.X) + FMath::Abs(RotationDelta.Y)) * 5.0f;
    AccumulatedShake += FrameShake;
    
    // 현재 위치 저장
    LastMousePosition = CurrentRotation2D;
    
    // Detach 체크
    if (AccumulatedShake >= ShakeThreshold)
    {
        UE_LOG(LogMonster, Warning, TEXT("%s shaken off! (Accumulated: %.1f >= Threshold: %.1f)"), 
            *GetName(), AccumulatedShake, ShakeThreshold);
        DetachFromPlayer();
        return;
    }
    
    // [DISABLED FOR DEMO] Debug shake visualization
#if 0
    if (GEngine && FrameShake > 0.1f)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, AccumulatedShake > ShakeThreshold * 0.7f ? FColor::Red : FColor::Yellow, FString::Printf(TEXT("Shake: %.0f/%.0f (Frame: %.1f)"), AccumulatedShake, ShakeThreshold, FrameShake));
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
        UE_LOG(LogMonster, Warning, TEXT("%s OnPerceptionUpdated: PotentialTarget SET (Noise detected)"), *GetName());
    }
    else
    {
        if (PotentialTarget == Player)
        {
            PotentialTarget = nullptr;
            UE_LOG(LogMonster, Log, TEXT("%s OnPerceptionUpdated: PotentialTarget CLEARED"), *GetName());
        }
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
            UE_LOG(LogMonster, Error, TEXT("%s DETECTED player via GAUGE (Gauge: %.1f) - TargetPlayer SET!"), 
                *GetName(), DetectionGauge);
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
                UE_LOG(LogMonster, Warning, TEXT("%s LOST player (gauge empty)"), *GetName());
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
            UE_LOG(LogMonster, Warning, TEXT("%s: Distance check in UpdateDetectionGauge - too far (%.1f)"), *GetName(), Distance);
        }
    }
}

// ========================================
// Carrier System (FlyingPlatform)
// ========================================

void AWallCrawler::AttachToCarrier(AFlyingPlatform* Platform)
{
    if (!Platform)
    {
        UE_LOG(LogMonster, Warning, TEXT("%s: AttachToCarrier - Platform is null"), *GetName());
        return;
    }

    bIsCarried = true;
    CarrierPlatform = Platform;

    // AI 중지
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->GetBrainComponent()->StopLogic(TEXT("Carried by Platform"));
        UE_LOG(LogMonster, Log, TEXT("%s: AI stopped - Carried by %s"), *GetName(), *Platform->GetName());
    }

    // CharacterMovement 중지
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
        Movement->SetComponentTickEnabled(false);
    }

    // Physics 끄기
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        RootPrimitive->SetSimulatePhysics(false);
        RootPrimitive->SetEnableGravity(false);
    }

    // Platform의 GrabPoint에 Attach
    USceneComponent* GrabPoint = nullptr;
    TArray<USceneComponent*> Components;
    Platform->GetComponents<USceneComponent>(Components);
    
    for (USceneComponent* Comp : Components)
    {
        if (Comp && Comp->GetName().Contains(TEXT("GrabPoint")))
        {
            GrabPoint = Comp;
            break;
        }
    }

    if (GrabPoint)
    {
        AttachToComponent(GrabPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        UE_LOG(LogMonster, Log, TEXT("%s: Attached to GrabPoint of %s"), *GetName(), *Platform->GetName());
    }
    else
    {
        // Fallback: Platform Root에 Attach
        AttachToActor(Platform, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        UE_LOG(LogMonster, Warning, TEXT("%s: GrabPoint not found, attached to Platform root"), *GetName());
    }

    // 상태 초기화
    bIsOnWall = false;
    bAttachedToPlayer = false;
    bIsStunned = false;
    TargetPlayer = nullptr;
    PotentialTarget = nullptr;
    DetectionGauge = 0.0f;
}

void AWallCrawler::DetachFromCarrier()
{
    if (!bIsCarried)
    {
        return;
    }

    UE_LOG(LogMonster, Log, TEXT("%s: Detaching from carrier"), *GetName());

    bIsCarried = false;
    CarrierPlatform = nullptr;

    // Detach from Platform
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // CharacterMovement 재활성화
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Falling);
        Movement->SetComponentTickEnabled(true);
    }

    // Physics 켜기 (자유낙하)
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        RootPrimitive->SetSimulatePhysics(true);
        RootPrimitive->SetEnableGravity(true);
    }

    // 착지 감지 시작 (OnActorHit 이벤트 활용)
    OnActorHit.AddDynamic(this, &AWallCrawler::OnCarrierLanded);
}

void AWallCrawler::OnCarrierLanded(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogMonster, Log, TEXT("%s: Landed!"), *GetName());

    // OnActorHit 이벤트 제거
    OnActorHit.RemoveDynamic(this, &AWallCrawler::OnCarrierLanded);

    // Physics 끄기
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        RootPrimitive->SetSimulatePhysics(false);
        RootPrimitive->SetEnableGravity(false);
        RootPrimitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
    }

    // CharacterMovement 복구
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Flying);
        Movement->GravityScale = 0.0f;
    }

    // 1초 기절
    bIsStunned = true;
    StunTimer = 1.0f;

    // 1초 후 AI 재시작 (Timer 사용)
    FTimerHandle StunTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        StunTimerHandle,
        [this]()
        {
            bIsStunned = false;
            StunTimer = 0.0f;

            // AI 재시작
            if (AAIController* AI = Cast<AAIController>(GetController()))
            {
                AI->GetBrainComponent()->StartLogic();
                UE_LOG(LogMonster, Log, TEXT("%s: AI restarted after landing"), *GetName());
            }
        },
        1.0f,
        false
    );
}