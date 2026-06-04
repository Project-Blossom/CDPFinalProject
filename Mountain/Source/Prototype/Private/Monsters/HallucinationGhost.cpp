// HallucinationGhost.cpp
#include "Monsters/HallucinationGhost.h"

#include "DownfallCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogGhost, Log, All);

AHallucinationGhost::AHallucinationGhost()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    SetRootComponent(MeshComp);

    // 충돌 없음 — 플레이어와 통과 가능
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetGenerateOverlapEvents(false);
}

void AHallucinationGhost::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();
    BaseZ         = SpawnLocation.Z;
    CurrentOpacity = GhostBaseOpacity;
    DiscoTimer     = FMath::RandRange(0.0f, DiscoInterval); // 스폰 시점 분산

    // DMI 생성
    if (MeshComp->GetMaterial(0))
    {
        GhostMID = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
        if (GhostMID)
        {
            GhostMID->SetScalarParameterValue(FName("Opacity"),      CurrentOpacity);
            GhostMID->SetScalarParameterValue(FName("BloodQuality"), BloodQuality);
            UE_LOG(LogGhost, Log, TEXT("HallucinationGhost: DMI 생성 완료"));
        }
    }

    CachedPlayer = Cast<ADownfallCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    SetNewWanderTarget();
    UE_LOG(LogGhost, Log, TEXT("HallucinationGhost: BeginPlay 완료 @ %s"), *SpawnLocation.ToString());
}

void AHallucinationGhost::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDespawning) return;

    ElapsedTime += DeltaTime;

    UpdateDespawnCheck();
    if (bIsDespawning) return;

    UpdateMovement(DeltaTime);
    UpdateLookAt(DeltaTime);
    UpdateDisco(DeltaTime);
}

// ─────────────────────────────────────────────────────────
// 이동 — 랜덤 배회 + 사인파 Z
// ─────────────────────────────────────────────────────────
void AHallucinationGhost::UpdateMovement(float DeltaTime)
{
    // 배회 목표 갱신
    if (!bHasWanderTarget ||
        FVector::DistXY(GetActorLocation(), WanderTarget) < WanderArrivalThreshold)
    {
        SetNewWanderTarget();
    }

    // XY: 목표 방향으로 이동
    const FVector CurrentPos = GetActorLocation();
    const FVector Dir = (WanderTarget - CurrentPos).GetSafeNormal2D();
    const FVector NewXY = CurrentPos + Dir * WanderSpeed * DeltaTime;

    // Z: 사인파 (BaseZ 기준 진동)
    const float SineZ = BaseZ + FMath::Sin(ElapsedTime * SineFrequency * 2.0f * PI) * SineAmplitude;

    SetActorLocation(FVector(NewXY.X, NewXY.Y, SineZ));
}

void AHallucinationGhost::SetNewWanderTarget()
{
    const float Angle  = FMath::RandRange(0.0f, 360.0f);
    const float Dist   = FMath::RandRange(WanderRadius * 0.3f, WanderRadius);
    const FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.0f
    );

    WanderTarget    = SpawnLocation + Offset;
    bHasWanderTarget = true;
}

// ─────────────────────────────────────────────────────────
// 플레이어 바라보기
// ─────────────────────────────────────────────────────────
void AHallucinationGhost::UpdateLookAt(float DeltaTime)
{
    ADownfallCharacter* Player = FindPlayer();
    if (!Player) return;

    const FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(
        GetActorLocation(), Player->GetActorLocation());
    const FRotator NewRot = FMath::RInterpTo(
        GetActorRotation(), TargetRot, DeltaTime, LookAtSpeed / 90.0f);
    SetActorRotation(FRotator(0.0f, NewRot.Yaw, 0.0f));
}

// ─────────────────────────────────────────────────────────
// 디스코 — DiscoInterval마다 Opacity 1.0 플래시 → 페이드 복귀
// ─────────────────────────────────────────────────────────
void AHallucinationGhost::UpdateDisco(float DeltaTime)
{
    if (!GhostMID) return;

    if (bDiscoFlashing)
    {
        // 플래시 후 GhostBaseOpacity로 페이드 아웃
        DiscoFadeTimer += DeltaTime;
        const float Alpha   = FMath::Clamp(DiscoFadeTimer / FMath::Max(DiscoFadeDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
        CurrentOpacity      = FMath::Lerp(1.0f, GhostBaseOpacity, Alpha);
        GhostMID->SetScalarParameterValue(FName("Opacity"), CurrentOpacity);

        if (Alpha >= 1.0f)
        {
            bDiscoFlashing = false;
            DiscoFadeTimer = 0.0f;
        }
    }
    else
    {
        DiscoTimer += DeltaTime;
        if (DiscoTimer >= DiscoInterval)
        {
            // 플래시 발동
            DiscoTimer     = 0.0f;
            DiscoFadeTimer = 0.0f;
            bDiscoFlashing = true;
            CurrentOpacity = 1.0f;
            GhostMID->SetScalarParameterValue(FName("Opacity"), 1.0f);
        }
    }
}

// ─────────────────────────────────────────────────────────
// 디스폰 조건 체크
// DespawnRadius 이내 접근 OR 스테이지 높이 초과 시 제거
// ─────────────────────────────────────────────────────────
void AHallucinationGhost::UpdateDespawnCheck()
{
    ADownfallCharacter* Player = FindPlayer();
    if (!Player) return;

    const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

    // 거리 기반 디스폰
    if (Dist < DespawnRadius)
    {
        UE_LOG(LogGhost, Log, TEXT("HallucinationGhost: Despawn (플레이어 접근 %.0fcm)"), Dist);
        bIsDespawning = true;
        Destroy();
        return;
    }

    // 스테이지 높이 초과 디스폰
    if (GetActorLocation().Z > StageHeightMax)
    {
        UE_LOG(LogGhost, Log, TEXT("HallucinationGhost: Despawn (높이 초과 Z=%.0f)"),
            GetActorLocation().Z);
        bIsDespawning = true;
        Destroy();
    }
}

ADownfallCharacter* AHallucinationGhost::FindPlayer() const
{
    if (CachedPlayer.IsValid()) return CachedPlayer.Get();
    return Cast<ADownfallCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}
