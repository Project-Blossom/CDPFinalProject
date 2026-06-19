// File: Source/Prototype/Public/Core/CliffSelectionPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "CliffSelectionPawn.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCliffSelectionHUDWidget;
class UCliffSelectionLoadingWidget;
class AMountainGenWorldActor;

/**
 * CliffSelection 레벨 플레이어 Pawn
 * - 좌/우 입력으로 인접 암벽으로 카메라 Yaw 회전 (FInterpTo)
 * - 카메라 상하 패닝은 ACameraAnimationActor와 동일한 방식 (Z축 이동 + 도달 시 리셋),
 *   현재 록온된 암벽의 CliffHeightCm을 기준으로 동작
 * - Enter 입력으로 현재 록온된 암벽의 Seed를 선택 확정 후 다음 스테이지로 이동
 */
UCLASS()
class PROTOTYPE_API ACliffSelectionPawn : public APawn
{
    GENERATED_BODY()

public:
    ACliffSelectionPawn();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    // ========== Components ==========

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CliffSelection|Camera")
    TObjectPtr<UCameraComponent> Camera;

protected:
    // ========== Input ==========

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Input")
    TObjectPtr<UInputMappingContext> CliffSelectionMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Input")
    TObjectPtr<UInputAction> MoveSelectionLeftAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Input")
    TObjectPtr<UInputAction> MoveSelectionRightAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Input")
    TObjectPtr<UInputAction> ConfirmSelectionAction;

    // R 키 - 암벽 리롤 (1회 한정, GameMode에서 처리)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Input")
    TObjectPtr<UInputAction> RerollAction;

    // ========== Camera Rotate (Lock-on) ==========

    // 카메라 Yaw 회전 보간 속도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Camera", meta = (ClampMin = "0.1"))
    float CameraInterpSpeed = 3.0f;

    // 록온된 암벽 정면 수직 거리 (cm) — 이 거리에서 암벽을 바라본다
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Camera", meta = (ClampMin = "100.0"))
    float CameraDistanceCm = 3200.f;

    // ========== Camera Vertical Pan (ACameraAnimationActor와 동일 로직) ==========

    // 카메라 상하 이동 속도 (cm/s)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Camera", meta = (ClampMin = "0.0"))
    float CameraPanSpeed = 5000.0f;

    // ========== Next Stage ==========

    // 다음 스테이지 레벨 이름 (Enter 확정 시 OpenLevel 대상)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|NextStage")
    FName NextStageLevelName;

    // ========== HUD ==========

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|UI")
    TSubclassOf<UCliffSelectionHUDWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|UI")
    TSubclassOf<UCliffSelectionLoadingWidget> LoadingWidgetClass;

private:
    // 현재 록온된 암벽 인덱스 (0/1/2 = 좌/중/우)
    int32 CurrentIndex = 1;

    // 목표 Yaw (FInterpTo 대상)
    float TargetYaw = 0.f;

    // 목표 XY 위치 (FInterpTo 대상) — 록온된 암벽 정면 CameraDistanceCm 지점
    FVector TargetXY = FVector::ZeroVector;

    // 현재 보간 중인지 여부 (회전 도중 입력 잠금에 사용 가능)
    bool bIsRotating = false;

    // 상하 패닝 - 시작 Z, 현재 이동 거리, 목표 거리(현재 암벽의 CliffHeightCm)
    float PanStartZ = 0.f;
    float PanCurrentDistance = 0.f;
    float PanTargetDistance = 80000.f;

    // 입력이 1회라도 들어왔는지 (초기 진입 시 HUD 안내 텍스트 표시 등에 사용)
    bool bHasGeneratedComplete = false;

    UPROPERTY()
    TObjectPtr<UCliffSelectionHUDWidget> HUDWidget;

    UPROPERTY()
    TObjectPtr<UCliffSelectionLoadingWidget> LoadingWidget;

    // 좌/우 입력 핸들러
    void OnMoveSelectionLeft(const FInputActionValue& Value);
    void OnMoveSelectionRight(const FInputActionValue& Value);

    // Enter 확정 핸들러
    void OnConfirmSelection(const FInputActionValue& Value);

    // R 키 리롤 핸들러
    void OnRerollPressed(const FInputActionValue& Value);

    // CurrentIndex 변경 -> 목표 Yaw 설정 + 패닝 타겟 거리 갱신 + HUD 갱신
    void StartCameraRotateTo(int32 NewIndex);

    // Tick에서 호출 - Yaw FInterpTo 처리
    void TickCameraRotate(float DeltaTime);

    // Tick에서 호출 - ACameraAnimationActor와 동일한 상하 패닝 처리
    void TickCameraPan(float DeltaTime);

    // 암벽 3개 생성 완료 후 호출 - 초기 록온 (인덱스 1 = 중앙) 및 HUD 초기화
    UFUNCTION()
    void HandleAllCliffsGenerated();

    // 현재 인덱스 기준 HUD 정보 갱신
    void UpdateHUDInfo();

    // 난이도 enum -> 표시용 문자열
    static FString GetDifficultyDisplayString(int32 StageIndex);

    // GameMode에서 스폰된 암벽 배열 가져오기 (캐시 없이 매번 조회)
    AMountainGenWorldActor* GetCliffAt(int32 Index) const;

    // Index 암벽 정면 수직 CameraDistanceCm 위치로 Pawn XY를 즉시 이동
    // (Z는 PanStartZ 유지, Yaw 보간 시작점은 유지됨)
    void SetCameraPositionForCliff(int32 Index);
};
