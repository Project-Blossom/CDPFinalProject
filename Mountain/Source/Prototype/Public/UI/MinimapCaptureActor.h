// MinimapCaptureActor.h
// FreeRunSetup 레벨에 배치하는 미니맵 캡처 전용 액터
// Generate & Preview 완료 후 FreeRunSetup 레벨 블루프린트에서 TriggerCapture() 호출
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MinimapCaptureActor.generated.h"

class ASceneCapture2D;

UCLASS()
class PROTOTYPE_API AMinimapCaptureActor : public AActor
{
    GENERATED_BODY()

public:
    AMinimapCaptureActor();

    // ShowFlags.SetWireframe 적용 여부
    // false → 레벨 ViewMode(CSG Wireframe)에 의존
    // true  → C++에서 강제 Wireframe ShowFlag 적용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    bool bForceWireframeShowFlag = false;

    /**
     * SceneCapture2D를 1회 캡처합니다.
     * FreeRunSetup 레벨 블루프린트에서
     * Generate & Preview 완료 직후 이 함수를 호출하세요.
     */
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void TriggerCapture();

protected:
    virtual void BeginPlay() override;
};
