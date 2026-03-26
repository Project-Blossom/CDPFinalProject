// File: Source/prototype/Public/DownfallCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Climbing/IClimbableSurface.h"
#include "Engine/PostProcessVolume.h"
#include "Item/InventoryComponent.h"
#include "DownfallCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UPhysicsConstraintComponent;
class UCameraComponent;
class UGripPointFinderComponent;
class UInventoryWidget;

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

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float GripStrength = 3000.0f; // Constraint 당기는 힘 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float GripDamping = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float ArmLength = 100.0f; // Constraint 제한 거리 (cm)

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float StaminaDrainPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float StaminaRecoverPerSecond = 15.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Cursor")
    float CursorInitialRepeatDelay = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Cursor")
    float CursorRepeatInterval = 0.12f;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Climbing")
    void OnHandGripped(bool bIsLeftHand, const FGripPointInfo& GripInfo);

    UFUNCTION(BlueprintImplementableEvent, Category = "Climbing")
    void OnHandReleased(bool bIsLeftHand);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
    void BP_UpdateInventoryMode(bool bInventoryOpen, int32 CursorIndex, int32 InHeldSlotIndex, bool bPreviewing);

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

    // New Glitch System (Material-based)
    UPROPERTY(EditAnywhere, Category = "VFX|Glitch")
    TObjectPtr<UMaterial> GlitchMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> GlitchMaterialInstance;

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

    // Inventory
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UInventoryComponent* GetInventory() const { return Inventory; }

    // UI 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Altitude")
    TSubclassOf<class UAltitudeWidget> AltitudeWidgetClass;

    UPROPERTY()
    TObjectPtr<UAltitudeWidget> AltitudeWidget;

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

    // Inventory State Helpers
    void RefreshInventoryUIState();
    void MoveInventoryCursor(int32 DX, int32 DY);
    bool TryMoveInventoryCursorFromInput(const FVector2D& MovementVector);
    void EnterInventoryFromCenter();
    void EnterInventoryFromHeldSlot();
    void CloseInventoryToEmptyHand();
    bool TryPickHeldItemFromCursor();
    bool TryUseHeldItem();
    bool IsValidInventorySlotIndex(int32 Index) const;
    bool IsPlaceableSlot(int32 Index) const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryComponent> Inventory;

#if !UE_BUILD_SHIPPING
    void DrawDebugInfo();
#endif

private:
    void UpdateGlitchEffect();
    void UpdateGlitchPatternSwitch(float DeltaTime);
    float CalculateNextSwitchInterval() const;
    void ApplyClimbingMappingContext();
    void UpdateAltitudeUI();
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
};