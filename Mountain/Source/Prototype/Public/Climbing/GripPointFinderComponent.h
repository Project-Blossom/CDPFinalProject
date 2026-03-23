#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Climbing/IClimbableSurface.h"
#include "GripPointFinderComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGripFinder, Log, All);

/*
 * 그립 포인트 탐지 컴포넌트
 * ProceduralMesh 지형 + IClimbableSurface 구현 액터를 모두 처리
 */
UCLASS(ClassGroup = (Climbing), meta = (BlueprintSpawnableComponent))
class PROTOTYPE_API UGripPointFinderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGripPointFinderComponent();

    /**
     * 카메라 방향으로 그립 포인트 찾기
     * @param CameraLocation 카메라 월드 위치
     * @param CameraForward  카메라 전방 벡터
     * @param OutGripInfo    찾은 그립 포인트 정보
     * @return 그립 포인트를 찾았으면 true
     */
    UFUNCTION(BlueprintCallable, Category = "Climbing")
    bool FindGripPoint(const FVector& CameraLocation, const FVector& CameraForward, FGripPointInfo& OutGripInfo);

    /**
     * 설치형 climbable actor가 새로 Spawn된 뒤 호출
     * (예: 앵커 설치 후)
     */
    UFUNCTION(BlueprintCallable, Category = "Climbing")
    void RefreshClimbableSurfaces();

    // 설정 파라미터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Search")
    float MaxReachDistance = 200.0f; // Ray 최대 거리 (cm)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Search")
    float MinSurfaceAngle = 30.0f; // 최소 경사각 (도)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Search")
    float MaxSurfaceAngle = 170.0f; // 최대 경사각 (도)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Search")
    float SurfaceSampleRadius = 50.0f; // 경사 계산용 샘플 반경 (cm)

protected:
    virtual void BeginPlay() override;

private:
    // 카메라 Ray로 첫 Hit 찾기
    bool FindInitialHitPoint(const FVector& Start, const FVector& Direction, FHitResult& OutHit);

    // Hit 지점 인근의 평균 경사 계산
    bool CalculateAverageSurfaceAngle(const FVector& HitPoint, const FVector& HitNormal, float& OutAngleDegrees, FVector& OutAverageNormal);

    // 경사각으로 그립 품질 계산
    float CalculateGripQuality(float SurfaceAngleDegrees) const;

    // 특수 climbable actor 캐시
    UPROPERTY(Transient)
    TArray<TScriptInterface<IClimbableSurface>> CachedClimbableSurfaces;

    void CacheClimbableSurfaces();
};