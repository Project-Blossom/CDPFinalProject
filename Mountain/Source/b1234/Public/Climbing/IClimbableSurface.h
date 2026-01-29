// File: Source/b1234/Public/Climbing/IClimbableSurface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IClimbableSurface.generated.h"

/**
 * 그립 포인트 정보
 */
USTRUCT(BlueprintType)
struct FGripPointInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector SurfaceNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadWrite)
	float GripQuality = 1.0f; // 0.0 (나쁨) ~ 1.0 (좋음)

	UPROPERTY(BlueprintReadWrite)
	float SurfaceAngleDegrees = 0.0f; // 경사각 (0~180도)

	UPROPERTY(BlueprintReadWrite)
	FVector AverageNormal = FVector::UpVector; // 인근 평균 법선

	UPROPERTY(BlueprintReadWrite)
	bool bIsValid = false;
};

/**
 * 등반 가능한 표면 인터페이스
 * MountainGen 플러그인 변경에 독립적
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UClimbableSurface : public UInterface
{
	GENERATED_BODY()
};

class B1234_API IClimbableSurface
{
	GENERATED_BODY()

public:
	/**
	 * 가장 가까운 그립 포인트 찾기
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbing")
	bool FindNearestGripPoint(const FVector& SearchOrigin, float SearchRadius, FGripPointInfo& OutGripInfo);
};