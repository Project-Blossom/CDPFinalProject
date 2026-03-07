#include "Monsters/FlyingAttacker.h"
#include "DownfallCharacter.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AFlyingAttacker::AFlyingAttacker()
{
    // 공격형은 더 빠르게
    FlightSpeed = 400.0f;
}

void AFlyingAttacker::BeginPlay()
{
    Super::BeginPlay();

    AttackState = EAttackState::Idle;
    LastAttackTime = -AttackCooldown;  // 시작 시 즉시 공격 가능
    
    // Territory 초기화
    InitializeTerritory();
    
    // Blackboard 초기화 (BT 사용 시)
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
        
        // LastAttackTime을 과거로 설정 (즉시 공격 가능)
        Blackboard->SetValueAsFloat("LastAttackTime", -100.0f);
        
        // bCanAttack을 true로 설정
        Blackboard->SetValueAsBool("bCanAttack", true);
        
        UE_LOG(LogMonster, Log, TEXT("%s Blackboard initialized (bCanAttack: true)"), *GetName());
    }

    UE_LOG(LogMonster, Log, TEXT("%s (Flying Attacker) ready to hunt"), *GetName());
}

void AFlyingAttacker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Behavior Tree가 실행 중이면 기존 로직 스킵
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->BrainComponent && AIController->BrainComponent->IsRunning())
    {
        // Behavior Tree가 제어 중
        return;
    }

    // 기존 로직 (Behavior Tree 없을 때만)
    UpdateAttackBehavior(DeltaTime);

#if !UE_BUILD_SHIPPING
    // 디버그 시각화
    FVector ActorLoc = GetActorLocation();
    
    // 상태별 색상
    FColor StateColor;
    FString StateText;
    switch (AttackState)
    {
    case EAttackState::Idle:
        StateColor = FColor::Green;
        StateText = TEXT("IDLE");
        break;
    case EAttackState::Pursuing:
        StateColor = FColor::Yellow;
        StateText = TEXT("PURSUING");
        break;
    case EAttackState::Charging:
        StateColor = FColor::Orange;
        StateText = TEXT("CHARGING");
        break;
    case EAttackState::Attacking:
        StateColor = FColor::Red;
        StateText = TEXT("ATTACKING");
        break;
    case EAttackState::Cooldown:
        StateColor = FColor::Blue;
        StateText = TEXT("COOLDOWN");
        break;
    }

    // 상태 표시
    DrawDebugString(
        GetWorld(),
        ActorLoc + FVector(0, 0, 100),
        StateText,
        nullptr,
        StateColor,
        0.0f,
        true
    );

    // 감지 범위 (추격 거리)
    if (AttackState == EAttackState::Idle)
    {
        DrawDebugSphere(
            GetWorld(),
            ActorLoc,
            PursuitDistance,
            16,
            FColor::Yellow,
            false,
            0.1f,
            0,
            1.0f
        );
    }

    // 공격 범위
    if (AttackState == EAttackState::Pursuing || AttackState == EAttackState::Charging)
    {
        DrawDebugSphere(
            GetWorld(),
            ActorLoc,
            AttackRange,
            12,
            FColor::Red,
            false,
            0.1f,
            0,
            2.0f
        );
    }

    // 돌진 경로 (Attacking 상태)
    if (AttackState == EAttackState::Attacking)
    {
        DrawDebugLine(
            GetWorld(),
            ActorLoc,
            AttackTargetLocation,
            FColor::Red,
            false,
            0.1f,
            0,
            5.0f
        );
        
        DrawDebugSphere(
            GetWorld(),
            AttackTargetLocation,
            50.0f,
            8,
            FColor::Orange,
            false,
            0.1f,
            0,
            2.0f
        );
    }

    // 플레이어 추적 선 (Pursuing)
    if (AttackState == EAttackState::Pursuing && TargetPlayer)
    {
        DrawDebugLine(
            GetWorld(),
            ActorLoc,
            TargetPlayer->GetActorLocation(),
            FColor::Yellow,
            false,
            0.1f,
            0,
            2.0f
        );
    }
#endif
}

void AFlyingAttacker::UpdateAttackBehavior(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // 상태별 행동
    switch (AttackState)
    {
    case EAttackState::Idle:
        {
            // 플레이어 감지 확인
            if (TargetPlayer)
            {
                float Distance = GetDistanceToPlayer();
                if (Distance <= PursuitDistance && Distance > 0)
                {
                    // 추격 시작
                    SetAttackState(EAttackState::Pursuing);
                    UE_LOG(LogMonster, Warning, TEXT("%s detected player at %.1f cm! PURSUING!"), *GetName(), Distance);
                }
                else
                {
                    // 거리가 너무 멀면 Territory 내 배회
                    UpdateIdlePatrol(DeltaTime);
                }
            }
            else
            {
                // 플레이어 없으면 Territory 내 배회
                UpdateIdlePatrol(DeltaTime);
            }
        }
        break;

    case EAttackState::Pursuing:
        {
            if (!TargetPlayer)
            {
                // 플레이어 놓침 - Territory로 복귀
                UE_LOG(LogMonster, Log, TEXT("%s lost player, returning to Territory"), *GetName());
                bHasPatrolTarget = false;  // 새 배회 위치 생성
                SetAttackState(EAttackState::Idle);
                break;
            }

            float Distance = GetDistanceToPlayer();
            
            // 플레이어가 감지 범위 밖으로 나가면 Territory로 복귀
            if (Distance > PursuitDistance)
            {
                UE_LOG(LogMonster, Warning, TEXT("%s player out of range (%.1fcm), returning to Territory"), 
                    *GetName(), Distance);
                bHasPatrolTarget = false;  // 새 배회 위치 생성
                SetAttackState(EAttackState::Idle);
                break;
            }
            
            if (Distance > PursuitDistance * 1.5f)
            {
                // 너무 멀어짐 - 포기
                SetAttackState(EAttackState::Idle);
                UE_LOG(LogMonster, Log, TEXT("%s lost player (too far)"), *GetName());
            }
            else if (Distance <= AttackRange && CanAttack())
            {
                // 공격 범위 진입
                Attack();
            }
            else
            {
                // 계속 추격 - 암벽 회피 OFF
                FVector PlayerLocation = TargetPlayer->GetActorLocation();
                FlyToLocation(PlayerLocation, FlightSpeed, false);  // ← 추가: false
            }
        }
        break;

    case EAttackState::Charging:
        {
            // 돌진 준비 중
            float ChargeElapsed = CurrentTime - ChargeStartTime;
            
            if (ChargeElapsed >= ChargeTime)
            {
                // 준비 완료 - 돌진 실행
                ExecuteCharge();
            }
            else
            {
                // 준비 중 - 플레이어 방향 바라보기
                if (TargetPlayer)
                {
                    FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                    if (!Direction.IsNearlyZero())
                    {
                        FRotator TargetRotation = Direction.Rotation();
                        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f);
                        SetActorRotation(NewRotation);
                    }
                }
            }
        }
        break;

    case EAttackState::Attacking:
        {
            // 돌진 중 - 암벽 회피 OFF
            FVector Direction = (AttackTargetLocation - GetActorLocation()).GetSafeNormal();
            FlyToLocation(AttackTargetLocation, AttackSpeed, false);  // ← 추가: false

            // 충돌 체크
            if (TargetPlayer)
            {
                float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
                if (Distance <= 100.0f && !bHasHitPlayer)  // 1m 이내
                {
                    OnAttackHit(TargetPlayer);
                }
            }

            // 목표 지점 도달 체크
            float DistanceToTarget = FVector::Dist(GetActorLocation(), AttackTargetLocation);
            if (DistanceToTarget <= 50.0f)
            {
                // 목표 지점 통과 - 빗나감
                if (!bHasHitPlayer)
                {
                    UE_LOG(LogMonster, Log, TEXT("%s attack missed!"), *GetName());
                }
                SetAttackState(EAttackState::Cooldown);
                LastAttackTime = CurrentTime;
            }
        }
        break;

    case EAttackState::Cooldown:
        {
            // 쿨다운 확인
            float CooldownElapsed = CurrentTime - LastAttackTime;
            
            if (CooldownElapsed >= AttackCooldown)
            {
                // 쿨다운 끝
                SetAttackState(EAttackState::Idle);
                UE_LOG(LogMonster, Log, TEXT("%s cooldown finished"), *GetName());
            }
            else
            {
                // 안전 거리 유지 - 암벽 회피 ON (후퇴)
                if (TargetPlayer)
                {
                    float Distance = GetDistanceToPlayer();
                    if (Distance < SafeDistance)
                    {
                        // 플레이어에게서 멀어지기
                        FVector AwayDirection = (GetActorLocation() - TargetPlayer->GetActorLocation()).GetSafeNormal();
                        FVector SafeLocation = GetActorLocation() + AwayDirection * 200.0f;
                        FlyToLocation(SafeLocation, FlightSpeed * 0.5f, true);  // ← 추가: true (또는 생략 가능)
                    }
                }
            }
        }
        break;
    }
}

void AFlyingAttacker::SetAttackState(EAttackState NewState)
{
    if (AttackState == NewState)
        return;

    EAttackState OldState = AttackState;
    AttackState = NewState;

    UE_LOG(LogMonster, Log, TEXT("%s state: %d -> %d"), 
        *GetName(), (int32)OldState, (int32)NewState);
}

void AFlyingAttacker::Attack()
{
    if (!CanAttack())
    {
        UE_LOG(LogMonster, Warning, TEXT("%s cannot attack (cooldown)"), *GetName());
        return;
    }

    StartCharging();
}

void AFlyingAttacker::StartCharging()
{
    SetAttackState(EAttackState::Charging);
    ChargeStartTime = GetWorld()->GetTimeSeconds();
    
    if (TargetPlayer)
    {
        AttackTargetLocation = TargetPlayer->GetActorLocation();
        UE_LOG(LogMonster, Log, TEXT("%s charging attack toward player!"), *GetName());
    }
}

void AFlyingAttacker::ExecuteCharge()
{
    SetAttackState(EAttackState::Attacking);
    bHasHitPlayer = false;
    
    UE_LOG(LogMonster, Warning, TEXT("%s CHARGING!"), *GetName());
}

void AFlyingAttacker::OnAttackHit(ADownfallCharacter* Player)
{
    if (!Player || bHasHitPlayer)
        return;

    bHasHitPlayer = true;
    
    // Insanity 증가
    Player->AddInsanity(InsanityDamage);
    
    UE_LOG(LogMonster, Warning, TEXT("%s HIT player! (+%.1f Insanity)"), 
        *GetName(), InsanityDamage);

    // 쿨다운 시작
    SetAttackState(EAttackState::Cooldown);
    LastAttackTime = GetWorld()->GetTimeSeconds();
}

bool AFlyingAttacker::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

float AFlyingAttacker::GetDistanceToPlayer() const
{
    if (!TargetPlayer)
        return -1.0f;

    return FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
}

void AFlyingAttacker::InitializeTerritory()
{
    TerritoryCenter = GetActorLocation();
    
    // 3D 옥탄트 방향 벡터 초기화 (8방향)
    SectorDirections.Empty();
    SectorMaxDistances.Empty();
    
    // 8개 옥탄트 (3D 공간 8등분)
    TArray<FVector> OctantDirections = {
        FVector(1, 1, 1),    // 0: +X +Y +Z (우상전)
        FVector(-1, 1, 1),   // 1: -X +Y +Z (좌상전)
        FVector(1, -1, 1),   // 2: +X -Y +Z (우하전)
        FVector(-1, -1, 1),  // 3: -X -Y +Z (좌하전)
        FVector(1, 1, -1),   // 4: +X +Y -Z (우상후)
        FVector(-1, 1, -1),  // 5: -X +Y -Z (좌상후)
        FVector(1, -1, -1),  // 6: +X -Y -Z (우하후)
        FVector(-1, -1, -1)  // 7: -X -Y -Z (좌하후)
    };
    
    for (int32 i = 0; i < 8; i++)
    {
        FVector Direction = OctantDirections[i].GetSafeNormal();
        SectorDirections.Add(Direction);
        
        // 이 방향으로 Line Trace
        FVector StartLocation = TerritoryCenter;
        FVector EndLocation = StartLocation + Direction * TerritoryRadius;
        
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        float MaxDistance = TerritoryRadius;
        
        if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, Params))
        {
            // 암벽 발견
            MaxDistance = FMath::Min(Hit.Distance, TerritoryRadius);
        }
        
        SectorMaxDistances.Add(MaxDistance);
        
        UE_LOG(LogMonster, Log, TEXT("%s Territory Octant %d (%.1f, %.1f, %.1f): %.1fcm"), 
            *GetName(), i, Direction.X, Direction.Y, Direction.Z, MaxDistance);
            
#if !UE_BUILD_SHIPPING
        // 디버그 시각화 (3D)
        DrawDebugLine(GetWorld(), StartLocation, StartLocation + Direction * MaxDistance, 
            FColor::Cyan, true, -1.0f, 0, 10.0f);
#endif
    }
    
    CurrentSector = GetCurrentSector();
    
    UE_LOG(LogMonster, Warning, TEXT("%s Territory initialized (3D sphere) at %s, Current Octant: %d"), 
        *GetName(), *TerritoryCenter.ToString(), CurrentSector);
}

int32 AFlyingAttacker::GetCurrentSector() const
{
    FVector CurrentLocation = GetActorLocation();
    FVector DirectionFromCenter = (CurrentLocation - TerritoryCenter).GetSafeNormal();
    
    if (DirectionFromCenter.IsNearlyZero())
    {
        return 0;  // 중심에 있으면 첫 번째 옥탄트
    }
    
    // 3D 옥탄트 결정 (XYZ 부호에 따라)
    int32 Octant = 0;
    
    if (DirectionFromCenter.X < 0) Octant += 1;  // -X
    if (DirectionFromCenter.Y < 0) Octant += 2;  // -Y
    if (DirectionFromCenter.Z < 0) Octant += 4;  // -Z
    
    // 옥탄트 매핑
    // 0: +X +Y +Z → 0
    // 1: -X +Y +Z → 1
    // 2: +X -Y +Z → 2
    // 3: -X -Y +Z → 3
    // 4: +X +Y -Z → 4
    // 5: -X +Y -Z → 5
    // 6: +X -Y -Z → 6
    // 7: -X -Y -Z → 7
    
    TArray<int32> OctantMap = {0, 1, 2, 3, 4, 5, 6, 7};
    
    return FMath::Clamp(Octant, 0, 7);
}

FVector AFlyingAttacker::GetRandomPatrolLocationInTerritory()
{
    int32 MaxAttempts = 20;
    int32 CurrentOctant = GetCurrentSector();
    
    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        // 현재 옥탄트가 아닌 다른 옥탄트 선택
        int32 TargetOctant = FMath::RandRange(0, 7);
        
        // 현재 옥탄트면 재선택
        if (TargetOctant == CurrentOctant)
        {
            TargetOctant = (TargetOctant + 1) % 8;
        }
        
        // 해당 옥탄트의 최대 거리와 방향
        float MaxDist = SectorMaxDistances[TargetOctant];
        FVector OctantDir = SectorDirections[TargetOctant];
        
        // 옥탄트 내 랜덤 거리 (최소 200cm 이상)
        float RandomDist = FMath::RandRange(200.0f, MaxDist * 0.9f);
        
        // 옥탄트 방향에서 약간의 변화 (구형 랜덤)
        FVector RandomOffset = FVector(
            FMath::RandRange(-0.3f, 0.3f),
            FMath::RandRange(-0.3f, 0.3f),
            FMath::RandRange(-0.3f, 0.3f)
        );
        
        FVector VariedDirection = (OctantDir + RandomOffset).GetSafeNormal();
        
        // 목표 위치 계산
        FVector TargetLocation = TerritoryCenter + VariedDirection * RandomDist;
        
        // IsLocationValid로 암벽 체크
        if (IsLocationValid(TargetLocation))
        {
            UE_LOG(LogMonster, Log, TEXT("%s Valid patrol location in octant %d (attempt %d)"), 
                *GetName(), TargetOctant, Attempt + 1);
            return TargetLocation;
        }
    }
    
    // 실패 시 안전한 위치 (Territory 중심 위쪽)
    UE_LOG(LogMonster, Warning, TEXT("%s Failed to find valid patrol location, using fallback"), 
        *GetName());
    return TerritoryCenter + FVector(0, 0, 300.0f);
}

bool AFlyingAttacker::IsInTerritory(const FVector& Location) const
{
    FVector DirectionFromCenter = (Location - TerritoryCenter).GetSafeNormal();
    float Distance = FVector::Dist(Location, TerritoryCenter);
    
    // 옥탄트 결정
    int32 Octant = 0;
    if (DirectionFromCenter.X < 0) Octant += 1;
    if (DirectionFromCenter.Y < 0) Octant += 2;
    if (DirectionFromCenter.Z < 0) Octant += 4;
    
    Octant = FMath::Clamp(Octant, 0, 7);
    
    // 해당 옥탄트의 최대 거리와 비교
    float MaxDist = SectorMaxDistances[Octant];
    
    return Distance <= MaxDist;
}

void AFlyingAttacker::UpdateIdlePatrol(float DeltaTime)
{
    // 목표가 없으면 Territory 내에서 새로 설정
    if (!bHasPatrolTarget)
    {
        CurrentPatrolTarget = GetRandomPatrolLocationInTerritory();
        bHasPatrolTarget = true;
        PatrolIdleTimer = 0.0f;
        CurrentSector = GetCurrentSector();  // 옥탄트 업데이트
        
        UE_LOG(LogMonster, Log, TEXT("%s new patrol target in Territory (Octant %d): %s"), 
            *GetName(), CurrentSector, *CurrentPatrolTarget.ToString());
    }

    // 목표로 이동
    FVector CurrentLocation = GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, CurrentPatrolTarget);

    if (Distance > PatrolArrivalThreshold)
    {
        // 아직 도착 안 함 - 계속 이동
        FlyToLocation(CurrentPatrolTarget, FlightSpeed, true);
    }
    else
    {
        // 도착 - 대기
        PatrolIdleTimer += DeltaTime;
        
        if (PatrolIdleTimer >= PatrolIdleWaitTime)
        {
            // 대기 끝 - 다음 목표 설정
            bHasPatrolTarget = false;
            UE_LOG(LogMonster, Log, TEXT("%s arrived at patrol target, choosing new target"), *GetName());
        }
    }
}