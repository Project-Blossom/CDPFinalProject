#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Interface.h"
#include "IClimbableSurface.generated.h"

/**
 * 그립 종류
 * - Surface     : 일반 ProceduralMesh 지형
 * - Anchor      : 고정 앵커 / 볼트 / 손잡이
 * - DynamicActor: 이동하는 플랫폼 / 몬스터 등
 */
UENUM(BlueprintType)
enum class EGripKind : uint8
{
    Surface      UMETA(DisplayName = "Surface"),
    Anchor       UMETA(DisplayName = "Anchor"),
    DynamicActor UMETA(DisplayName = "DynamicActor")
};

/**
 * 그립 포인트 정보
 */
USTRUCT(BlueprintType)
struct FGripPointInfo
{
    GENERATED_BODY()

    // 실제 손이 붙을 월드 위치
    UPROPERTY(BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    // 표면 Normal
    UPROPERTY(BlueprintReadWrite)
    FVector SurfaceNormal = FVector::UpVector;

    // 그립 품질
    // 일반 지형: 0.1 ~ 1.0
    // 앵커: 특수값 5.0 사용
    UPROPERTY(BlueprintReadWrite)
    float GripQuality = 1.0f;

    // 경사각 (일반 지형용)
    UPROPERTY(BlueprintReadWrite)
    float SurfaceAngleDegrees = 0.0f;

    // 주변 평균 Normal (일반 지형용)
    UPROPERTY(BlueprintReadWrite)
    FVector AverageNormal = FVector::UpVector;

    // 유효한 그립인지
    UPROPERTY(BlueprintReadWrite)
    bool bIsValid = false;

    // 어떤 액터를 잡은 건지 직접 전달
    // 일반 지형이면 nullptr
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> SourceActor = nullptr;

    // 어떤 종류의 그립인지
    // Surface     : 일반 지형
    // Anchor      : 고정 앵커
    // DynamicActor: 움직이는 플랫폼/액터
    UPROPERTY(BlueprintReadWrite)
    EGripKind GripKind = EGripKind::Surface;
};

/**
 * 등반 가능한 표면 인터페이스
 * ProceduralMesh 기반 일반 지형이 아니라,
 * 개별 액터(플랫폼, 앵커 등) 쪽 확장용
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UClimbableSurface : public UInterface
{
    GENERATED_BODY()
};

class PROTOTYPE_API IClimbableSurface
{
    GENERATED_BODY()

public:
    /**
     * 가장 가까운 그립 포인트 찾기
     * @param SearchOrigin  탐색 시작 위치
     * @param SearchRadius  탐색 반경
     * @param OutGripInfo   결과 그립 정보
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbing")
    bool FindNearestGripPoint(const FVector& SearchOrigin, float SearchRadius, FGripPointInfo& OutGripInfo);
};