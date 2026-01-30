#include "PrototypeCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputCoreTypes.h"

#include "ClimbDebugComponent.h"

APrototypeCharacter::APrototypeCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	ClimbDebugComp = CreateDefaultSubobject<UClimbDebugComponent>(TEXT("ClimbDebugComp"));

	DebugIMC = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_DebugRuntime"));
	IA_DebugEnterFP = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugEnterFP"));
	IA_DebugExitFP = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugExitFP"));
	IA_DebugTrace = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugTrace"));

	IA_DebugEnterFP->ValueType = EInputActionValueType::Boolean;
	IA_DebugExitFP->ValueType = EInputActionValueType::Boolean;
	IA_DebugTrace->ValueType = EInputActionValueType::Boolean;
}

void APrototypeCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocallyControlled()) return;

	SetupDebugEnhancedInput();
	AddDebugMappingContext();
}

void APrototypeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		}
		if (MouseLookAction)
		{
			EIC->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		}

		// Debug bindings
		if (IA_DebugEnterFP) EIC->BindAction(IA_DebugEnterFP, ETriggerEvent::Started, this, &APrototypeCharacter::DbgEnterFP);
		if (IA_DebugExitFP)  EIC->BindAction(IA_DebugExitFP, ETriggerEvent::Started, this, &APrototypeCharacter::DbgExitFP);
		if (IA_DebugTrace)   EIC->BindAction(IA_DebugTrace, ETriggerEvent::Started, this, &APrototypeCharacter::DbgTrace);
	}
}

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();
	DoMove(V.X, V.Y);
}

void APrototypeCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();
	DoLook(V.X, V.Y);
}

void APrototypeCharacter::DoMove(float Right, float Forward)
{
	if (!Controller) return;

	const FRotator Rot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, Rot.Yaw, 0.f);

	const FVector Fwd = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Rgt = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(Fwd, Forward);
	AddMovementInput(Rgt, Right);
}

void APrototypeCharacter::DoLook(float Yaw, float Pitch)
{
	if (!Controller) return;

	AddControllerYawInput(Yaw);
	AddControllerPitchInput(Pitch);
}

void APrototypeCharacter::DoJumpStart()
{
	Jump();
}

void APrototypeCharacter::DoJumpEnd()
{
	StopJumping();
}

void APrototypeCharacter::SetupDebugEnhancedInput()
{
	if (bDebugInputSetupDone) return;
	bDebugInputSetupDone = true;

	if (!DebugIMC || !IA_DebugEnterFP || !IA_DebugExitFP || !IA_DebugTrace) return;

	DebugIMC->MapKey(IA_DebugEnterFP, EKeys::Q);
	DebugIMC->MapKey(IA_DebugExitFP, EKeys::E);
	DebugIMC->MapKey(IA_DebugTrace, EKeys::LeftMouseButton);
}

void APrototypeCharacter::AddDebugMappingContext()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	if (DebugIMC)
	{
		Subsystem->AddMappingContext(DebugIMC, 100);
	}
}

// ------------------------
// Debug handlers
// ------------------------

void APrototypeCharacter::DbgEnterFP()
{
	if (ClimbDebugComp) ClimbDebugComp->EnterDebugFP();
}

void APrototypeCharacter::DbgExitFP()
{
	if (ClimbDebugComp) ClimbDebugComp->ExitDebugFP();
}

void APrototypeCharacter::DbgTrace()
{
	if (ClimbDebugComp) ClimbDebugComp->TraceAndPrintAngle();
}
