// File: Source/prototype/Public/DownfallCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Climbing/IClimbableSurface.h"
#include "DownfallCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UPhysicsConstraintComponent;
class UCameraComponent;
class UGripPointFinderComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogDownFall, Log, All);

/**
 * 손 상태
 */
UENUM(BlueprintType)
enum class EHandState : uint8
{
    Free       UMETA(DisplayName = "Free"),
    Gripping   UMETA(DisplayName = "Gripping")
};

/**
 * 손 데이터
 */
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

    // ========================================
    // Components
    // ========================================
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

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

    // ========================================
    // Enhanced Input
    // ========================================
    
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

    // ========================================
    // Settings
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float GripStrength = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float GripDamping = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics")
    float ArmLength = 100.0f; // Constraint 제한 거리 (cm)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float StaminaDrainPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stamina")
    float StaminaRecoverPerSecond = 15.0f;

    // ========================================
    // State
    // ========================================
    
    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    FHandData LeftHand;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    FHandData RightHand;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing|State")
    bool bIsClimbing = false;

    // ========================================
    // Events
    // ========================================
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Climbing")
    void OnHandGripped(bool bIsLeftHand, const FGripPointInfo& GripInfo);

    UFUNCTION(BlueprintImplementableEvent, Category = "Climbing")
    void OnHandReleased(bool bIsLeftHand);

protected:
    // ========================================
    // Input Handlers
    // ========================================
    
    void OnGrabLeftStarted(const FInputActionValue& Value);
    void OnGrabLeftCompleted(const FInputActionValue& Value);
    void OnGrabRightStarted(const FInputActionValue& Value);
    void OnGrabRightCompleted(const FInputActionValue& Value);
    void OnLook(const FInputActionValue& Value);
    void OnMove(const FInputActionValue& Value);
    void OnJumpStarted(const FInputActionValue& Value); 
    void OnJumpCompleted(const FInputActionValue& Value);

    // ========================================
    // Grip Logic
    // ========================================
    
    void TryGrip(bool bIsLeftHand);
    void ReleaseGrip(bool bIsLeftHand);
    void SetupConstraint(UPhysicsConstraintComponent* Constraint, const FVector& TargetLocation);
    void BreakConstraint(UPhysicsConstraintComponent* Constraint);
    
    // ========================================
    // Stamina
    // ========================================
    
    void UpdateStamina(float DeltaTime);
    float GetStaminaDrainRate(const FHandData& Hand) const;
    
    // ========================================
    // State
    // ========================================
    
    void UpdateClimbingState();
    bool AreBothHandsFree() const;

    // ========================================
    // Debug
    // ========================================
    
#if !UE_BUILD_SHIPPING
    void DrawDebugInfo();
#endif
};