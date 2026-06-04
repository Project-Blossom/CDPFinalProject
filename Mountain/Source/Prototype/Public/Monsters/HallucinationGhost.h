// HallucinationGhost.h
// Insanity 80+ 조건에서 소환되는 환각 몬스터
// FlyingAttacker 대체. 충돌 없음. 사인파 이동 + 디스코 플래시 + DespawnRadius 기반 디스폰
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HallucinationGhost.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class ADownfallCharacter;

UCLASS()
class PROTOTYPE_API AHallucinationGhost : public AActor
{
    GENERATED_BODY()

public:
    AHallucinationGhost();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── 이동 설정 ────────────────────────────────────────────
    // 배회 범위 반경 (스폰 위치 기준)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Movement")
    float WanderRadius = 700.0f;

    // 배회 이동 속도 (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Movement")
    float WanderSpeed = 150.0f;

    // 배회 목표 도달 임계값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Movement")
    float WanderArrivalThreshold = 80.0f;

    // 사인파 Z 진폭 (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Movement")
    float SineAmplitude = 60.0f;

    // 사인파 주파수 (Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Movement")
    float SineFrequency = 0.5f;

    // 플레이어 바라보기 회전 속도 (도/초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Movement")
    float LookAtSpeed = 90.0f;

    // ── 디스폰 설정 ──────────────────────────────────────────
    // 플레이어가 이 거리 이내로 접근 시 즉시 디스폰
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Despawn")
    float DespawnRadius = 300.0f;

    // 스테이지 최대 높이 초과 시 디스폰 (CliffTotalHeight 연동)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Despawn")
    float StageHeightMax = 80000.0f;

    // ── 디스코 설정 ──────────────────────────────────────────
    // 디스코 플래시 발동 간격 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Disco")
    float DiscoInterval = 3.0f;

    // 플래시 후 기본 투명도로 복귀 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Disco")
    float DiscoFadeDuration = 0.5f;

    // 평상시 투명도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Disco")
    float GhostBaseOpacity = 0.25f;

    // ── 머티리얼 ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|Visual")
    float BloodQuality = 0.5f;   // 머티리얼 파라미터 (기획서 기본값)

    // ── 런타임 상태 (읽기 전용) ───────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Ghost|State")
    bool bIsDespawning = false;

private:
    // 컴포넌트
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> MeshComp;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> GhostMID;

    // 캐시
    TWeakObjectPtr<ADownfallCharacter> CachedPlayer;

    // 이동 상태
    FVector SpawnLocation;
    FVector WanderTarget;
    bool bHasWanderTarget = false;
    float ElapsedTime = 0.0f;
    float BaseZ = 0.0f;          // 사인파 기준 Z

    // 디스코 상태
    float DiscoTimer = 0.0f;
    float DiscoFadeTimer = 0.0f;
    bool bDiscoFlashing = false;
    float CurrentOpacity = 0.25f;

    // 내부 함수
    void UpdateMovement(float DeltaTime);
    void UpdateLookAt(float DeltaTime);
    void UpdateDisco(float DeltaTime);
    void UpdateDespawnCheck();
    void SetNewWanderTarget();
    ADownfallCharacter* FindPlayer() const;
};
