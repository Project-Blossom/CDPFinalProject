#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraAnimationActor.generated.h"

/**
 * FreeRunSetup 레벨에서 카메라를 자동으로 위로 이동시키는 Actor
 * MountainGen의 CliffHeight만큼 천천히 위로 이동하고 루프
 */
UCLASS()
class PROTOTYPE_API ACameraAnimationActor : public AActor
{
    GENERATED_BODY()

public:
    ACameraAnimationActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 카메라 이동 속도 (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    float CameraSpeed = 5000.0f; // 5000cm/s = 50m/s

    // 목표 이동 거리 (cm) - 에디터에서 수정 가능
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation", meta = (ClampMin = "100.0"))
    float TargetMoveDistance = 80000.0f; // 기본값: CliffHeightCm

private:
    // 카메라 Actor
    UPROPERTY()
    TObjectPtr<class ACameraActor> CameraActor;

    // MountainGen Actor
    UPROPERTY()
    TObjectPtr<class AMountainGenWorldActor> MountainActor;

    // 시작 위치
    FVector StartLocation;

    // 현재 이동 거리
    float CurrentDistance;

    // 초기화 완료 여부
    bool bInitialized;

    // 초기화
    void Initialize();

    // 재시작
    void Restart();
};
