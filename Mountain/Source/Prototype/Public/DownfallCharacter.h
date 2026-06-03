// File: Source/prototype/Public/DownfallCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Climbing/IClimbableSurface.h"
#include "Engine/PostProcessVolume.h"
#include "Item/InventoryComponent.h"
#include "TimerManager.h"
#include <limits>
#include "DownfallCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UPhysicsConstraintComponent;
class UCameraComponent;
class UGripPointFinderComponent;
class UInventoryWidget;
class UNiagaraComponent;
class UNiagaraSystem;
class ADirectionalLight;
class AExponentialHeightFog;
class AStaticMeshActor;
class ASceneCapture2D;
class UMinimapWidget;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_LOG_CATEGORY_EXTERN(LogDownFall, Log, All);

// 손 상태
UENUM(BlueprintType)
enum class EHandState : uint8
{
    Free       UMETA(DisplayName = "Free"),
    Gripping   UMETA(DisplayName = "Gripping")
};

// 손 데이터
USTRUCT(BlueprintType)
struct FHandData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EHandState State = EHandState::Free;

    UPROPERTY(BlueprintReadOnly)
    FGripPointInfo CurrentGrip;

    UPROPERTY(BlueprintReadOnly)
    float GripStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Stamina = 100.0f;

    UPROPERTY()
    AActor* GrippedActor = nullptr;
};

// 그립 모드
// - SurfaceGrip : 일반 지형 등반
// - AnchorGrip  : 고정 앵커 / 볼트 / 손잡이
// - DynamicGrip : 이동 플랫폼 / 몬스터 등
UENUM(BlueprintType)
enum class EGripMode : uint8
{
    None         UMETA(DisplayName = "None"),
    SurfaceGrip  UMETA(DisplayName = "SurfaceGrip"),
    AnchorGrip   UMETA(DisplayName = "AnchorGrip"),
    DynamicGrip  UMETA(DisplayName = "DynamicGrip")
};

UENUM(BlueprintType)
enum class EItemUseState : uint8
{
    None              UMETA(DisplayName = "None"),
    InventoryOpen     UMETA(DisplayName = "InventoryOpen"),
    HoldingItem       UMETA(DisplayName = "HoldingItem"),
    PlacementPreview  UMETA(DisplayName = "PlacementPreview")
};

UCLASS()
class PROTOTYPE_API ADownfallCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ADownfallCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Debug|Movement")
    void ToggleFlyMode();

    UFUNCTION(BlueprintCallable, Category = "SafetyLine")
    bool TryAttachSafetyLineFromLookTarget(float TraceDistanceCm = 600.0f);

    // InventoryComponent 등 외부 클래스에서 앵커 액터를 직접 지정해 연결할 때 사용
    UFUNCTION(BlueprintCallable, Category = "SafetyLine")
    bool AttachSafetyLineToBolt(AActor* AnchorActor);

    UFUNCTION(BlueprintCallable, Category = "SafetyLine")
    void DetachSafetyLine(bool bBreakBolt = false);

    UFUNCTION(BlueprintCallable, Category = "SafetyLine")
    bool BeginUsingAnchorSlot(int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "SafetyLine")
    bool IsAnchorInUse() const { return bSafetyLineAttached && ActiveUsingAnchorSlotIndex != INDEX_NONE; }

    UFUNCTION(BlueprintPure, Category = "SafetyLine")
    int32 GetActiveUsingAnchorSlotIndex() const { return ActiveUsingAnchorSlotIndex; }

    UFUNCTION(BlueprintPure, Category = "Inventory|UI")
    bool IsInventorySlotUsing(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Inventory|UI")
    int32 GetDisplayedInventoryCountAt(int32 Index) const;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<class UPostProcessComponent> PostProcessComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UAIPerceptionStimuliSourceComponent> StimuliSource;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing")
    TObjectPtr<UGripPointFinderComponent> GripFinder;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing|Hands")
    TObjectPtr<USceneComponent> LeftHandTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing|Hands")
    TObjectPtr<USceneComponent> RightHandTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing|Physics")
    TObjectPtr<UPhysicsConstraintComponent> LeftHandConstraint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing|Physics")
    TObjectPtr<UPhysicsConstraintComponent> RightHandConstraint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SafetyLine")
    TObjectPtr<UPhysicsConstraintComponent> SafetyLineConstraint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SafetyLine|Visual")
    TObjectPtr<USplineComponent> SafetyLineSpline;

    // Enhanced Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> ClimbingMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> GrabLeftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> GrabRightAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> DebugInsanityAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> UseItemAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ToggleInventoryAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> PauseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> UtilityUseAction;

    UPROPERTY(BlueprintReadOnly, Category = "Debug|Movement")
    bool bDebugFlyMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Movement", meta = (ClampMin = "100.0"))
    float DebugFlySpeed = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Movement", meta = (ClampMin = "100.0"))
    float DebugFlyVerticalSpeed = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PlatformAbductionCheckInterval = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float AltitudeUpdateInterval = 0.10f;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float GripStrength = 3000.0f; // Constraint 당기는 힘 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float GripDamping = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float ArmLength = 50.0f; // Constraint 제한 거리 (cm)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insanity")
    float Insanity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insanity")
    float MaxInsanity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insanity")
    float InsanityDecayRate = 0.1f; // 초당 자연 감소 (1초에 0.1씩)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insanity")
    float InsanityGrowthRate = 0.1f; // 혼란 상태일 때 초당 증가 (1초에 0.1씩)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insanity")
    float InsanityThreshold = 70.0f; // 혼란 효과 시작 지점

    UPROPERTY(BlueprintReadOnly, Category = "Insanity")
    bool bIsConfused = false;

    // Altitude Widget Glitch 임계값 (80)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Insanity")
    float AltitudeGlitchThreshold = 80.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Insanity")
    bool bWasAboveGlitchThreshold = false; // 이전 프레임에 80 이상이었는지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float StaminaDrainPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float StaminaRecoverPerSecond = 15.0f;

    // 경사각 구간 경계값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Thresholds", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
    float CeilingAngleThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Thresholds", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
    float SteepAngleThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Thresholds", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
    float VerticalAngleThreshold = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Thresholds", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
    float OverhangAngleThreshold = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Thresholds", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
    float SteepOverhangAngleThreshold = 150.0f;

    // 경사각 구간별 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "5.0"))
    float CeilingAngleMultiplier = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "5.0"))
    float SteepAngleMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "5.0"))
    float VerticalAngleMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "5.0"))
    float OverhangAngleMultiplier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "5.0"))
    float SteepOverhangAngleMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Angle Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "5.0"))
    float FloorAngleMultiplier = 0.5f;

    // 품질 구간 경계값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Thresholds", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "5.0"))
    float VeryPoorQualityThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Thresholds", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "5.0"))
    float PoorQualityThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Thresholds", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "5.0"))
    float GoodQualityThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Thresholds", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "10.0"))
    float AnchorQualityThreshold = 5.0f;

    // 품질 구간별 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.5", UIMax = "5.0"))
    float VeryPoorQualityMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.5", UIMax = "5.0"))
    float PoorQualityMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.5", UIMax = "5.0"))
    float NormalQualityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Multipliers", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.5", UIMax = "5.0"))
    float GoodQualityMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina|Quality Multipliers", meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
    float AnchorQualityMultiplier = 0.2f;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    FHandData LeftHand;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    FHandData RightHand;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    EGripMode GripMode = EGripMode::None;

    // Anchor State
    UPROPERTY(BlueprintReadOnly, Category = "Climbing|Anchor")
    TObjectPtr<AActor> CurrentAnchorActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|Anchor")
    FVector CurrentAnchorPointWorld = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|Anchor")
    FVector CurrentAnchorNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Anchor")
    float AnchorBodyBackOffset = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Anchor")
    float AnchorBodyDownOffset = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Anchor")
    float AnchorAlignSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Anchor")
    float AnchorRotationSpeed = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|Anchor")
    bool bAnchorGripLeftHand = false;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|Anchor")
    bool bAnchorGripRightHand = false;

    // Item State
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    EItemUseState ItemUseState = EItemUseState::None;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    int32 InventoryCursorIndex = 12; // 5x5 중앙

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    int32 HeldSlotIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    int32 ActiveUsingAnchorSlotIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    int32 EquippedUtilitySlotIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Cursor")
    float CursorInitialRepeatDelay = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Cursor")
    float CursorRepeatInterval = 0.12f;

    // Safety Line
    UPROPERTY(BlueprintReadOnly, Category = "SafetyLine")
    bool bSafetyLineAttached = false;

    UPROPERTY(BlueprintReadOnly, Category = "SafetyLine")
    bool bSafetyLineTriggered = false;

    UPROPERTY(BlueprintReadOnly, Category = "SafetyLine")
    bool bSafetyLineConstraintEngaged = false;

    UPROPERTY(BlueprintReadOnly, Category = "SafetyLine")
    TObjectPtr<AActor> ActiveSafetyBolt = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "SafetyLine")
    FVector SafetyLineAnchorWorld = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "SafetyLine")
    bool bSafetyLineRetrieveArmed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine", meta = (ClampMin = "100.0"))
    float SafetyLineCurrentLengthCm = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine", meta = (ClampMin = "100.0"))
    float SafetyLineInitialLengthCm = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine", meta = (ClampMin = "50.0"))
    float SafetyLineMinLengthCm = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine", meta = (ClampMin = "0.0"))
    float SafetyLineSlackCm = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine", meta = (ClampMin = "0.0"))
    float SafetyLineEngageToleranceCm = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine", meta = (ClampMin = "0.0"))
    float SafetyLineAutoRetrieveDistanceCm = 1100.0f;

    // Safety Line Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual")
    TObjectPtr<UStaticMesh> SafetyLineVisualMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual")
    TObjectPtr<UMaterialInterface> SafetyLineVisualMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual", meta = (ClampMin = "0.05"))
    float SafetyLineVisualUpdateInterval = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual", meta = (ClampMin = "10.0"))
    float SafetyLineVisualSegmentLengthCm = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual", meta = (ClampMin = "1.0"))
    float SafetyLineVisualThickness = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual", meta = (ClampMin = "0.0"))
    float SafetyLineVisualSagCm = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual", meta = (ClampMin = "1", ClampMax = "16"))
    int32 SafetyLineVisualMaxSegments = 6;

    // 1인칭 시점에서 로프가 화면 중앙을 가리지 않도록,
    // 로프의 플레이어 쪽 시각 끝점을 카메라 기준 화면 위쪽으로 보정한다.
    // 실제 물리 제약 위치는 그대로 두고 시각 표현만 변경한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual|FirstPerson")
    bool bUseFirstPersonSafetyLineVisualOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual|FirstPerson", meta = (ClampMin = "0.0"))
    float SafetyLineVisualCameraForwardOffsetCm = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual|FirstPerson")
    float SafetyLineVisualCameraRightOffsetCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual|FirstPerson")
    float SafetyLineVisualCameraUpOffsetCm = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafetyLine|Visual|FirstPerson", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SafetyLineVisualOpacity = 0.35f;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Climbing")
    void OnHandGripped(bool bIsLeftHand, const FGripPointInfo& GripInfo);

    UFUNCTION(BlueprintImplementableEvent, Category = "Climbing")
    void OnHandReleased(bool bIsLeftHand);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
    void BP_UpdateInventoryMode(bool bInventoryOpen, int32 CursorIndex, int32 InHeldSlotIndex, bool bPreviewing);

    UFUNCTION(BlueprintPure, Category = "Inventory|UI")
    bool IsInventorySlotUtilityEquipped(int32 Index) const;

    // Stamina
    void UpdateStamina(float DeltaTime);
    float GetStaminaDrainRate(const FHandData& Hand) const;

    UFUNCTION(BlueprintCallable, Category = "Climbing|Stamina")
    bool CanRestoreStamina() const;

    UFUNCTION(BlueprintCallable, Category = "Climbing|Stamina")
    bool RestoreStamina(float Amount);

    // Insanity
    UFUNCTION(BlueprintCallable, Category = "Insanity")
    void AddInsanity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "AI|Stealth")
    void ActivateMonsterSenseBlock(float DurationSeconds = 15.0f);

    UFUNCTION(BlueprintCallable, Category = "AI|Stealth")
    void EndMonsterSenseBlock();

    UFUNCTION(BlueprintPure, Category = "AI|Stealth")
    bool IsMonsterSenseBlocked() const { return bMonsterSenseBlocked; }

    UFUNCTION(BlueprintCallable, Category = "Insanity")
    void UpdateInsanity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Insanity")
    void UpdateInsanityEffects();

    // Attach Desaturation VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ShowAttachDesaturation(float Amount = 0.8f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void HideAttachDesaturation();

    void UpdateAttachDesaturation(float DeltaTime);

    // Rain Hallucination VFX — PP + Niagara 동시 On/Off
    UFUNCTION(BlueprintCallable, Category = "VFX|RainVFX")
    void ActivateRainVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|RainVFX")
    void DeactivateRainVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|BlizzardVFX")
    void ActivateBlizzardVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|BlizzardVFX")
    void DeactivateBlizzardVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|BloodMoonVFX")
    void ActivateBloodMoonVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|BloodMoonVFX")
    void DeactivateBloodMoonVFX();

    // New Glitch System (Material-based)
    UPROPERTY(EditAnywhere, Category = "VFX|Glitch")
    TObjectPtr<UMaterial> GlitchMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> GlitchMaterialInstance;

    // Dirt Mask System (Lens dirt effect)
    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask")
    TObjectPtr<UMaterial> DirtMaskMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DirtMaskMaterialInstance;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DirtIntensity = 1.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float DirtBlurOffset = 1.5f;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float DirtTintStrength = 1.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLightResponse = 1.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLightThreshold = 0.6f;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float DirtLightSoftness = 0.2f;

    UPROPERTY(EditAnywhere, Category = "VFX|DirtMask", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DirtExposure = 1.0f;

    // Edge Blur System (3-Sample blur)
    UPROPERTY(EditAnywhere, Category = "VFX|EdgeBlur")
    TObjectPtr<UMaterial> EdgeBlurMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> EdgeBlurMaterialInstance;

    UPROPERTY(EditAnywhere, Category = "VFX|EdgeBlur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlurStart = 0.80f;

    UPROPERTY(EditAnywhere, Category = "VFX|EdgeBlur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlurEnd = 0.95f;

    UPROPERTY(EditAnywhere, Category = "VFX|EdgeBlur", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BlurStrength = 1.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|EdgeBlur", meta = (ClampMin = "0.0", ClampMax = "0.05"))
    float BlurOffset = 0.002f;

    UPROPERTY(EditAnywhere, Category = "VFX|Glitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoiseIntensity = 0.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|Glitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentNoiseIntensity = 0.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|Glitch")
    float PatternSwitchBaseInterval = 2.0f; // 기본 전환 간격 (초)

    UPROPERTY(EditAnywhere, Category = "VFX|Glitch")
    float PatternSwitchRandomness = 1.0f; // 랜덤 변동 폭

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Glitch")
    int32 CurrentPattern = 0; // 0, 1, 2

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Glitch")
    float TimeSinceLastSwitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Glitch")
    float NextSwitchTime = 2.0f;

    // Lens Distortion System (Material-based)
    UPROPERTY(EditAnywhere, Category = "VFX|LensDistortion")
    TObjectPtr<UMaterial> LensDistortionMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> LensDistortionMaterialInstance;

    UPROPERTY(EditAnywhere, Category = "VFX|LensDistortion")
    float BaseK1 = 0.0f; // Insanity 0일 때 K1 값

    UPROPERTY(EditAnywhere, Category = "VFX|LensDistortion")
    float MaxK1 = -0.3f; // Insanity 100일 때 K1 값 (음수 = Barrel Distortion)

    UPROPERTY(EditAnywhere, Category = "VFX|LensDistortion")
    float K2 = 0.0f; // 2차 왜곡 계수 (고정값)

    UPROPERTY(BlueprintReadOnly, Category = "VFX|LensDistortion")
    float CurrentK1 = 0.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|ChromaticAberration")
    float BaseChromaticAberration = 0.0f; // Insanity 0일 때

    UPROPERTY(EditAnywhere, Category = "VFX|ChromaticAberration")
    float MaxChromaticAberration = 5.0f; // Insanity 100일 때

    UPROPERTY(BlueprintReadOnly, Category = "VFX|ChromaticAberration")
    float CurrentChromaticAberration = 0.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|FilmGrain")
    float BaseFilmGrainIntensity = 0.0f; // Insanity 0일 때

    UPROPERTY(EditAnywhere, Category = "VFX|FilmGrain")
    float MaxFilmGrainIntensity = 1.0f; // Insanity 100일 때

    UPROPERTY(BlueprintReadOnly, Category = "VFX|FilmGrain")
    float CurrentFilmGrainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, Category = "VFX|Vignette")
    TObjectPtr<UMaterial> VignetteMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> VignetteMaterialInstance;

    UPROPERTY(EditAnywhere, Category = "VFX|Vignette")
    float VignetteGradientStart = 0.9f;

    UPROPERTY(EditAnywhere, Category = "VFX|Vignette")
    float VignetteGradientEnd = 0.93f;

    UPROPERTY(EditAnywhere, Category = "VFX|Vignette")
    float VignetteShiftAmount = 0.05f; // 카메라 움직임에 따른 이동 강도

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Vignette")
    FVector2D CurrentVignetteOffset = FVector2D::ZeroVector;

    // Attach Desaturation VFX
    UPROPERTY(EditAnywhere, Category = "VFX")
    TObjectPtr<UMaterial> AttachDesaturationMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DesaturationMaterialInstance;

    UPROPERTY(EditAnywhere, Category = "VFX")
    float DesaturationAmount = 0.8f;

    UPROPERTY(EditAnywhere, Category = "VFX")
    float DesaturationFadeSpeed = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float CurrentDesaturationAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bShowingDesaturation = false;

    // ── Rain Hallucination VFX ──────────────────────────────────
    // 트리거: ClimbingElapsedTime >= RainTriggerTime 시 즉시 On
    // 해제 조건: 없음 (스테이지 내 유지)
    // 기획서: Downfall Mountain 빗방울 환각 VFX

    // Blueprint에서 M_PP_RainDrop 할당
    UPROPERTY(EditAnywhere, Category = "VFX|RainVFX")
    TObjectPtr<UMaterial> RainDropMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> RainDropMaterialInstance;

    // Blueprint에서 Niagara 에셋 할당 (미할당 시 PP 효과만 활성화)
    UPROPERTY(EditAnywhere, Category = "VFX|RainVFX")
    TObjectPtr<UNiagaraSystem> RainNiagaraSystem;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> RainNiagaraComponent;

    // 트리거 시간 (초). 에디터에서 조정 가능. 기본 300초(5분)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "0.0"))
    float RainTriggerTime = 300.0f;

    // 누적 등반 시간 (bIsClimbing == true 인 동안 증가)
    UPROPERTY(BlueprintReadOnly, Category = "VFX|RainVFX")
    float ClimbingElapsedTime = 0.0f;

    // 현재 Rain VFX 활성 상태
    UPROPERTY(BlueprintReadOnly, Category = "VFX|RainVFX")
    bool bRainActive = false;

    // ── Blizzard Hallucination VFX ──────────────────────────────
    // Stage 2 이상에서 활성화. Rain과 동시 발생 없음.
    // 기획서: Downfall Mountain 폭설 환각 VFX

    UPROPERTY(EditAnywhere, Category = "VFX|BlizzardVFX")
    TObjectPtr<UMaterial> FrostMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> FrostMaterialInstance;

    UPROPERTY(EditAnywhere, Category = "VFX|BlizzardVFX")
    TObjectPtr<UNiagaraSystem> BlizzardNiagaraSystem;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> BlizzardNiagaraComponent;

    // 트리거 시간 (초). 기본 300초 = 5분
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "0.0"))
    float BlizzardTriggerTime = 300.0f;

    // PP 서리 블렌딩 가중치 (0.0~1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlizzardBlendWeight = 0.15f;

    // FrostIntensity 즉시 전환 (Lerp 없음 — 기획서 명시)
    // PP 가중치 Lerp 지속 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "0.1"))
    float BlizzardLerpDuration = 1.5f;

    // AutoExposureBias 어두워지는 강도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "-5.0", ClampMax = "0.0"))
    float BlizzardDarkenBias = -1.0f;

    // AutoExposureBias Lerp 지속 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "0.1"))
    float BlizzardDarkenLerpDuration = 4.0f;

    // SpawnRate 최솟값 (활성화 직후)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "1.0"))
    float BlizzardSpawnRateMin = 50.0f;

    // SpawnRate 최댓값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "1.0"))
    float BlizzardSpawnRateMax = 400.0f;

    // SpawnRate 점진 강화 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX",
        meta = (ClampMin = "1.0"))
    float BlizzardIntensifyDuration = 20.0f;

    // ── Wind 기반 스폰 위치 계산 ─────────────────────────────────
    // Niagara Wind Force 값과 동기화해서 입력
    // 스폰 위치 = 캐릭터 위치 - WindDir.Normalized * BlizzardSpawnDistance
    // → 파티클이 Wind에 밀려 캐릭터 근처를 통과하는 궤도로 배치

    // Niagara Wind Force XYZ 값 (Niagara 에디터의 Wind Force 모듈 기본값과 동일하게 입력)
    // SetVariableVec3로 "Wind Speed" User Parameter에 그대로 전달됨
    // 각 에미터의 WindSpeedScale Random Range는 Niagara 내부에서 별도 적용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX|WindSpawn")
    FVector BlizzardWindForce = FVector(80.f, 30.f, -300.f);

    // 스폰 위치 계산 전용 평균 스케일
    // BlowingParticles(3.33~10, avg≈6.67) + HangingParticulates(10~25, avg≈17.5) 평균 ≈ 12
    // Niagara WindSpeedScale Random Range와 별개로 스폰 거리 계산에만 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX|WindSpawn",
        meta = (ClampMin = "0.1"))
    float BlizzardWindSpeedScale = 12.f;

    // Wind 방향 반대쪽으로 얼마나 멀리 스폰할지 (cm)
    // 값이 클수록 더 멀리서 생성되어 오래 날아옴
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BlizzardVFX|WindSpawn",
        meta = (ClampMin = "100.0"))
    float BlizzardSpawnDistance = 15000.f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|BlizzardVFX")
    bool bBlizzardActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|BlizzardVFX")
    float BlizzardCurrentWeight = 0.0f;

    // ── Blood Moon Event VFX ─────────────────────────────────────
    // Stage 3: DirectionalLight 색상 흰색 → 붉은색 Lerp + ExponentialHeightFog 붉은 안개
    // 기획서: Downfall Mountain 블러드문 이벤트 VFX

    // 트리거 시간 (초). ClimbingElapsedTime 기반
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.0"))
    float BloodMoonTriggerTime = 300.0f;

    // 광원 색상 전환 Lerp 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.1"))
    float BloodMoonLerpDuration = 10.0f;

    // 전환 목표 광원 색상 (붉은달 색)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonLightColor = FLinearColor(1.0f, 0.1f, 0.0f, 1.0f);

    // 전환 목표 안개 색상
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonFogColor = FLinearColor(0.4f, 0.02f, 0.0f, 1.0f);

    // 안개 밀도 목표값 (기존 대비 추가 밀도)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.0"))
    float BloodMoonFogDensity = 0.02f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|BloodMoonVFX")
    bool bBloodMoonActive = false;

    // 하늘 붉게 물들이는 PostProcess 머티리얼 (M_PP_BloodMoonSky)
    // SceneDepth로 하늘 픽셀 감지 후 붉은 오버레이 적용
    UPROPERTY(EditAnywhere, Category = "VFX|BloodMoonVFX")
    TObjectPtr<UMaterial> BloodMoonSkyMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> BloodMoonSkyMaterialInstance;

    // 하늘 붉은 오버레이 최대 강도 (0~1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BloodMoonSkyIntensity = 0.4f;

    // ── MoonScale (MI_Sky) Lerp ──────────────────────────────────
    // SM_Sky 액터에 태그 "BloodMoonSky" 부여 필요
    // BeginPlay에서 자동 탐색 후 Dynamic Material Instance 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FName SkySphereActorTag = FName("BloodMoonSky");

    // 정상 상태 달 크기 (MI_Sky MoonScale 기본값과 동일하게 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.0"))
    float BloodMoonNormalMoonScale = 0.1f;

    // 블러드문 달 크기 목표값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.0"))
    float BloodMoonTargetMoonScale = 0.3f;

    // ── Volumetric Cloud 색상 전환 ────────────────────────────────
    // 레벨에 VolumetricCloud 액터 배치 필요
    // 클라우드 머티리얼에 "CloudColor" VectorParameter 추가 필요
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonNormalCloudColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // 블러드문 달 색상 틴트 (M_Sky MoonTintColor 파라미터 제어)
    // 기본값: 흰색 (원본 유지), 목표: 붉은색
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonNormalMoonTintColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonMoonTintColor = FLinearColor(1.0f, 0.15f, 0.05f, 1.0f);

    // 블러드문 구름 색상 목표값 (청록색 계열)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonCloudColor = FLinearColor(0.0f, 0.3f, 0.4f, 1.0f);

    // 전환 시작 기준값 — 에디터에서 레벨의 기본 안개/광원 색상과 맞춰 입력
    // BeginPlay 시 컴포넌트에서 읽지 않고 이 값을 Lerp 시작점으로 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonNormalLightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX")
    FLinearColor BloodMoonNormalFogColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|BloodMoonVFX",
        meta = (ClampMin = "0.0"))
    float BloodMoonNormalFogDensity = 0.005f;

    // PP 머티리얼 블렌딩 가중치 (0.0~1.0, 에디터에서 세기 조절)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainDropBlendWeight = 0.1f;

    // Activate 시 Weight가 목표값까지 Lerp되는 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "0.1"))
    float RainDropLerpDuration = 1.0f;

    // 활성화 후 SpawnRate 가 최대까지 증가하는 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "1.0"))
    float RainIntensifyDuration = 30.0f;

    // SpawnRate 최솟값 (활성화 직후)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "1.0"))
    float RainSpawnRateMin = 10.0f;

    // SpawnRate 최댓값 (RainIntensifyDuration 경과 후)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "1.0"))
    float RainSpawnRateMax = 200.0f;

    // 스폰 박스 XY 반경 (cm). 플레이어 주변 스테이지 규모로 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "100.0"))
    float RainBoxExtentXY = 3000.0f;

    // 비 내릴 때 화면 어두워지는 강도 (EV 단위, 음수 = 어두워짐)
    // -1.0 = 절반 밝기, -2.0 = 1/4 밝기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "-5.0", ClampMax = "0.0"))
    float RainDarkenBias = -1.5f;

    // AutoExposureBias Lerp 지속 시간 (초) — RainDropLerpDuration과 독립
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|RainVFX",
        meta = (ClampMin = "0.1"))
    float RainDarkenLerpDuration = 3.0f;

    // 현재 실제 적용 중인 Weight (Lerp 진행값, 디버그용)
    UPROPERTY(BlueprintReadOnly, Category = "VFX|RainVFX")
    float RainDropCurrentWeight = 0.0f;

    // Inventory
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UInventoryComponent* GetInventory() const { return Inventory; }

    // UI 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Altitude")
    TSubclassOf<class UAltitudeWidget> AltitudeWidgetClass;

    UPROPERTY()
    TObjectPtr<UAltitudeWidget> AltitudeWidget;

    // ── Wireframe Minimap (WBP_AltitudeIndicator 대체) ─────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Minimap")
    TSubclassOf<class UMinimapWidget> MinimapWidgetClass;

    UPROPERTY()
    TObjectPtr<class UMinimapWidget> MinimapWidget;

    // 미니맵 Color Tint 기본값 (정상 상태)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    FLinearColor MinimapNormalTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Blizzard 발동 시 미니맵 Color Tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    FLinearColor MinimapBlizzardTint = FLinearColor(0.6f, 0.85f, 1.0f, 1.0f);

    // BloodMoon 발동 시 미니맵 Color Tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    FLinearColor MinimapBloodMoonTint = FLinearColor(1.0f, 0.2f, 0.1f, 1.0f);

    // SceneCapture2D Actor — 레벨에 배치 후 태그 "MinimapCapture" 지정
    // BeginPlay에서 자동 탐색 → CaptureScene() 1회 호출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    FName SceneCaptureActorTag = FName("MinimapCapture");

    // [테스트] ShowFlags.SetWireframe(true) 후 캡처 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    bool bCaptureWithWireframeShowFlag = false;

    // 캡처 시 조명 영향 여부 (false = Unlit, 와이어프레임 미니맵에 권장)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    bool bCaptureWithLighting = false;

    // 캡처 시 그림자 포함 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap")
    bool bCaptureWithShadows = false;

    // 미니맵 Orthographic 캡처 범위 (cm) — SceneCapture2D OrthoWidth와 동일하게 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap",
        meta = (ClampMin = "100.0"))
    float MinimapCaptureWidth = 50000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap",
        meta = (ClampMin = "100.0"))
    float MinimapCaptureHeight = 50000.f;

    // 암벽 전체 크기 (UV 계산 기준)
    // MountainGenWorldActor의 CliffTotalWidth/Height와 동일하게 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap",
        meta = (ClampMin = "100.0"))
    float CliffTotalWidth = 50000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap",
        meta = (ClampMin = "100.0"))
    float CliffTotalHeight = 50000.f;

    // 미니맵 표시 범위 (cm) — 캐릭터 중심 기준 표시할 반경
    // 기획서 기본값: 10000.0f (100m)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Minimap",
        meta = (ClampMin = "100.0"))
    float MinimapViewRange = 10000.f;

    // Pause Menu
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Pause")
    TSubclassOf<class UPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY()
    TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

    // Hand
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hands|Mesh")
    TObjectPtr<USkeletalMeshComponent> LeftHandMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hands|Mesh")
    TObjectPtr<USkeletalMeshComponent> RightHandMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hands|Position")
    FVector LeftHandRestPosition = FVector(30.0f, -40.0f, -30.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hands|Position")
    FVector RightHandRestPosition = FVector(30.0f, 40.0f, -30.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hands|Animation")
    float HandMoveSpeed = 10.0f;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> LeftHandMaterialInstance;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> RightHandMaterialInstance;

    UPROPERTY(EditDefaultsOnly, Category = "Hand Mesh")
    float HandShakeIntensity = 4.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Hand Mesh")
    float HandShakeSpeed = 20.0f;

    float HandShakeTimer = 0.0f;

protected:
    // Input Handlers
    void OnGrabLeftStarted(const FInputActionValue& Value);
    void OnGrabLeftCompleted(const FInputActionValue& Value);
    void OnGrabRightStarted(const FInputActionValue& Value);
    void OnGrabRightCompleted(const FInputActionValue& Value);
    void OnLook(const FInputActionValue& Value);
    void OnMove(const FInputActionValue& Value);
    void OnJumpStarted(const FInputActionValue& Value);
    void OnJumpCompleted(const FInputActionValue& Value);
    void OnUseItemTriggered(const FInputActionValue& Value);
    void OnToggleInventoryTriggered(const FInputActionValue& Value);
    void OnPauseTriggered(const FInputActionValue& Value);
    void OnUtilityUseTriggered(const FInputActionValue& Value);
    void OnDestroyInventoryItemPressed();

    // Debug
    void OnDebugInsanity(const FInputActionValue& Value);

    // Grip Logic
    void TryGrip(bool bIsLeftHand);
    void ReleaseGrip(bool bIsLeftHand);
    void SetupConstraint(UPhysicsConstraintComponent* Constraint, const FVector& TargetLocation);
    void SetupConstraintToActor(UPhysicsConstraintComponent* Constraint, AActor* TargetActor, const FVector& GripLocation);
    void BreakConstraint(UPhysicsConstraintComponent* Constraint);

    // Anchor Grip Logic
    void EnterAnchorGrip(bool bIsLeftHand, const FGripPointInfo& GripInfo);
    void UpdateAnchorGrip(float DeltaTime);
    void ExitAnchorGrip(bool bIsLeftHand);
    bool IsAnchorGripActive() const;

    // Grip Query Helpers
    bool HasAnchorGrip() const;
    bool HasSurfaceGrip() const;
    bool HasDynamicGrip() const;
    const FHandData* GetPrimaryMovementHand() const;
    float GetAnchorAssistMoveScale() const;
    float GetAnchorAssistInterpSpeed() const;

    // State
    void UpdateClimbingState();
    void CheckForPlatformAbduction(); // 납치 체크
    void AbductByPlatform(bool bIsLeftHand, class AFlyingPlatform* Platform); // 납치 실행
    bool AreBothHandsFree() const;

    // Safety Line
    void UpdateSafetyLine(float DeltaTime);
    void EngageSafetyLineConstraint();
    void DisengageSafetyLineConstraint();
    void RefreshSafetyLineConstraint();
    FVector GetSafetyLineAnchorLocation() const;
    FVector ResolveSafetyLineAnchorLocation(const AActor* AnchorActor) const;
    bool IsSafetyLineTaut() const;
    void FinishUsingAnchor(bool bConsumeOneUse);

    // Inventory State Helpers
    void RefreshInventoryUIState();
    void MoveInventoryCursor(int32 DX, int32 DY);
    bool TryMoveInventoryCursorFromInput(const FVector2D& MovementVector);
    void EnterInventoryFromCenter();
    void EnterInventoryFromHeldSlot();
    void CloseInventoryToEmptyHand();
    bool TryPickHeldItemFromCursor();
    bool TryUseHeldItem();
    bool TryUseEquippedUtility();
    bool TryDestroyInventoryItemAtCursor();
    bool CanDestroyInventorySlot(int32 Index) const;
    bool IsUtilityEquipSlot(int32 Index) const;
    bool BeginEquippingUtilitySlot(int32 SlotIndex);
    void ClearEquippedUtilitySlot();
    bool IsValidInventorySlotIndex(int32 Index) const;
    bool IsPlaceableSlot(int32 Index) const;

    UPROPERTY()
    FTimerHandle MonsterSenseBlockTimerHandle;

    UPROPERTY(Transient)
    TMap<FName, float> UtilityCooldownEndTimes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Stealth", meta = (AllowPrivateAccess = "true"))
    bool bMonsterSenseBlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryComponent> Inventory;

#if !UE_BUILD_SHIPPING
    void DrawDebugInfo();
#endif

private:
    void UpdateGlitchEffect();
    void UpdateGlitchPatternSwitch(float DeltaTime);
    void UpdateLensDistortionEffect();
    void UpdateVignetteEffect(float DeltaTime);
    void UpdateDirtMaskEffect();
    void UpdateEdgeBlurEffect();
    void UpdateRainVFX(float DeltaTime);  // ClimbingElapsedTime 누적 + 트리거 체크
    void UpdateBlizzardVFX(float DeltaTime);
    void UpdateBloodMoonVFX(float DeltaTime); // Stage 3 분기 + 광원/안개 Lerp
    void ApplyDirtMaskParameters(bool bForce = false);
    void ApplyEdgeBlurParameters(bool bForce = false);
    void RefreshLowFrequencyUpdates();
    void StartLowFrequencyUpdatesIfNeeded();
    void StopLowFrequencyUpdatesIfPossible();
    void UpdateSafetyLineVisual();
    void ClearSafetyLineVisual();
    void EnsureSafetyLineVisualMeshPool(int32 RequiredSegments);
    FVector GetSafetyLineVisualPlayerEndLocation() const;
    float CalculateNextSwitchInterval() const;
    void ApplyClimbingMappingContext();
    void UpdateAltitudeUI();
    void UpdateMinimapUI();        // 플레이어 마커 UV + Color Tint Lerp
    void StartMinimapTintLerp(const FLinearColor& TargetTint, float Duration); // 이벤트 발동 시 호출
    class AMountainGenWorldActor* FindMountainGenActor();
    float GetGroundHeight() const;
    float InitialGroundHeight = 0.0f;
    void UpdateHandPositions(float DeltaTime);
    FVector GetHandTargetPosition(bool bIsLeftHand) const;
    void UpdateHandStaminaVisuals(float DeltaTime);
    void UpdateHandMaterial(UMaterialInstanceDynamic* MaterialInstance, float Stamina);
    void UpdateHandShake(USkeletalMeshComponent* HandMesh, float Stamina, float DeltaTime, const FVector& RestPosition);

private:
    FVector2D LastCursorInputDir = FVector2D::ZeroVector;
    bool bCursorInputHeld = false;
    float NextCursorRepeatTime = 0.0f;

    FTimerHandle LowFrequencyUpdateTimerHandle;
    TWeakObjectPtr<class AMountainGenWorldActor> CachedMountainActor;

    // Minimap SceneCapture2D 캐시
    TWeakObjectPtr<class ASceneCapture2D>  CachedSceneCapture;

    // Minimap Color Tint Lerp 내부 상태
    FLinearColor MinimapCurrentTint = FLinearColor::White;
    FLinearColor MinimapLerpStartTint = FLinearColor::White;
    FLinearColor MinimapLerpTargetTint = FLinearColor::White;
    bool  bMinimapTintLerping = false;
    float MinimapTintLerpElapsed = 0.0f;
    float MinimapTintLerpDuration = 10.0f; // BloodMoonLerpDuration과 공유

    UPROPERTY(Transient)
    TArray<TObjectPtr<USplineMeshComponent>> SafetyLineSplineMeshes;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> SafetyLineSplineMaterialInstances;

    float CachedDirtIntensity = -std::numeric_limits<float>::max();
    float CachedDirtBlurOffset = -std::numeric_limits<float>::max();
    float CachedDirtTintStrength = -std::numeric_limits<float>::max();
    float CachedDirtLightResponse = -std::numeric_limits<float>::max();
    float CachedDirtLightThreshold = -std::numeric_limits<float>::max();
    float CachedDirtLightSoftness = -std::numeric_limits<float>::max();
    float CachedDirtExposure = -std::numeric_limits<float>::max();

    float CachedBlurStart = -std::numeric_limits<float>::max();
    float CachedBlurEnd = -std::numeric_limits<float>::max();
    float CachedBlurStrength = -std::numeric_limits<float>::max();
    float CachedBlurOffset = -std::numeric_limits<float>::max();

    // Rain VFX Lerp 내부 상태
    bool  bRainWeightLerping = false;
    float RainDropLerpElapsed = 0.0f;
    float RainDropLerpStartWeight = 0.0f;
    float RainDropLerpTargetWeight = 0.0f;

    // Rain SpawnRate 점진 강화 내부 상태
    float RainElapsedSinceActivation = 0.0f;

    // AutoExposureBias Lerp 내부 상태
    bool  bRainDarkenLerping = false;
    float RainDarkenLerpElapsed = 0.0f;
    float RainDarkenLerpStartBias = 0.0f;
    float RainDarkenLerpTargetBias = 0.0f;

    // Blizzard VFX 내부 상태
    bool  bBlizzardWeightLerping = false;
    float BlizzardLerpElapsed = 0.0f;
    float BlizzardLerpStartWeight = 0.0f;
    float BlizzardElapsedSinceActivation = 0.0f;
    bool  bBlizzardDarkenLerping = false;
    float BlizzardDarkenLerpElapsed = 0.0f;
    float BlizzardDarkenLerpStartBias = 0.0f;
    float BlizzardDarkenLerpTargetBias = 0.0f;

    // BloodMoon VFX 내부 상태
    float BloodMoonElapsed = 0.0f;  // Lerp 경과 시간
    FLinearColor BloodMoonStartLightColor = FLinearColor::White;  // 전환 시작 광원 색상
    FLinearColor BloodMoonStartFogColor = FLinearColor::Black;  // 전환 시작 안개 색상
    float BloodMoonStartFogDensity = 0.0f;                 // 전환 시작 안개 밀도

    // 레벨에서 찾은 광원/안개 캐시 (BeginPlay에서 탐색, Null 허용)
    TWeakObjectPtr<ADirectionalLight>      CachedMoonLight;
    TWeakObjectPtr<AExponentialHeightFog>  CachedHeightFog;

    // MoonScale DMI — SM_Sky 액터에서 동적 생성
    TWeakObjectPtr<AStaticMeshActor>       CachedSkySphere;
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic>   SkySphereMID;
    float                                  BloodMoonStartMoonScale = 0.1f;
    FLinearColor                           BloodMoonStartMoonTintColor = FLinearColor::White;

    // Volumetric Cloud DMI — AActor로 저장, FindComponentByClass로 접근
    TWeakObjectPtr<AActor>                 CachedVolumetricCloud;
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic>   CloudMaterialMID;
    FLinearColor                           BloodMoonStartCloudColor = FLinearColor::White;

    // ── Minimap 내부 함수 ──────────────────────────────────────
    // 암벽 바운드 자동 감지 → SceneCapture2D 위치/OrthoWidth 자동 설정
    // CliffTotalWidth/Height가 기본값(50000)이면 바운드에서 자동 읽어 설정
    void AutoConfigureMinimapCapture();
};