#include "Monsters/FlyingAttacker.h"
#include "DownfallCharacter.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "BrainComponent.h"

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
                    // 거리가 너무 멀면 배회
                    UpdateIdlePatrol(DeltaTime);
                }
            }
            else
            {
                // 플레이어 없으면 배회
                UpdateIdlePatrol(DeltaTime);
            }
        }
        break;

    case EAttackState::Pursuing:
        {
            if (!TargetPlayer)
            {
                // 플레이어 놓침
                SetAttackState(EAttackState::Idle);
                break;
            }

            float Distance = GetDistanceToPlayer();
            
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

void AFlyingAttacker::UpdateIdlePatrol(float DeltaTime)
{
    // 목표가 없으면 새로 설정
    if (!bHasPatrolTarget)
    {
        CurrentPatrolTarget = GetRandomPatrolLocation();
        bHasPatrolTarget = true;
        PatrolIdleTimer = 0.0f;
        
        UE_LOG(LogMonster, Log, TEXT("%s new patrol target: %s"), *GetName(), *CurrentPatrolTarget.ToString());
    }

    // 목표로 이동 - 암벽 회피 ON (배회)
    FVector CurrentLocation = GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, CurrentPatrolTarget);

    if (Distance > PatrolArrivalThreshold)
    {
        // 아직 도착 안 함 - 계속 이동
        FlyToLocation(CurrentPatrolTarget, FlightSpeed, true);  // ← 추가: true (또는 생략 가능)
    }
    else
    {
        // 도착 - 대기
        PatrolIdleTimer += DeltaTime;
        
        if (PatrolIdleTimer >= PatrolIdleWaitTime)
        {
            // 대기 끝 - 다음 목표 설정
            bHasPatrolTarget = false;
            UE_LOG(LogMonster, Log, TEXT("%s arrived, choosing new target"), *GetName());
        }
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