#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PrototypeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UClimbDebugComponent;

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS(abstract)
class PROTOTYPE_API APrototypeCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb", meta = (AllowPrivateAccess = "true"))
	UClimbDebugComponent* ClimbDebugComp;

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

private:
	UPROPERTY(Transient)
	UInputMappingContext* DebugIMC;

	UPROPERTY(Transient)
	UInputAction* IA_DebugEnterFP;

	UPROPERTY(Transient)
	UInputAction* IA_DebugExitFP;

	UPROPERTY(Transient)
	UInputAction* IA_DebugTrace;

	bool bDebugInputSetupDone = false;

public:
	APrototypeCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

private:
	// Debug handlers
	void DbgEnterFP();
	void DbgExitFP();
	void DbgTrace();

	// runtime setup
	void SetupDebugEnhancedInput();
	void AddDebugMappingContext();
};