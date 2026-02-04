// File: Source/prototype/Private/DownfallCharacter.cpp
#include "DownfallCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Climbing/GripPointFinderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogDownFall);

ADownfallCharacter::ADownfallCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Grip Finder
    GripFinder = CreateDefaultSubobject<UGripPointFinderComponent>(TEXT("GripFinder"));

    // Hand Targets (IK용 - Phase 2에서 사용)
    LeftHandTarget = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandTarget"));
    LeftHandTarget->SetupAttachment(FirstPersonCamera);
    LeftHandTarget->SetRelativeLocation(FVector(50.0f, -30.0f, -20.0f));

    RightHandTarget = CreateDefaultSubobject<USceneComponent>(TEXT("RightHandTarget"));
    RightHandTarget->SetupAttachment(FirstPersonCamera);
    RightHandTarget->SetRelativeLocation(FVector(50.0f, 30.0f, -20.0f));

    // Physics Constraints
    LeftHandConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("LeftHandConstraint"));
    LeftHandConstraint->SetupAttachment(GetCapsuleComponent());
    LeftHandConstraint->SetDisableCollision(true);

    RightHandConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("RightHandConstraint"));
    RightHandConstraint->SetupAttachment(GetCapsuleComponent());
    RightHandConstraint->SetDisableCollision(true);

    // Movement
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    bUseControllerRotationYaw = false;

    // Capsule Physics (초기에는 비활성화)
    GetCapsuleComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
    GetCapsuleComponent()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    GetCapsuleComponent()->SetSimulatePhysics(false);
    GetCapsuleComponent()->SetEnableGravity(true);
    GetCapsuleComponent()->SetLinearDamping(0.5f);
    GetCapsuleComponent()->SetAngularDamping(0.8f);
}

void ADownfallCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input 등록
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (IsValid(ClimbingMappingContext))
            {
                Subsystem->AddMappingContext(ClimbingMappingContext, 0);
                UE_LOG(LogDownFall, Log, TEXT("ClimbingMappingContext registered"));
            }
            else
            {
                UE_LOG(LogDownFall, Warning, TEXT("ClimbingMappingContext is null - assign in Blueprint"));
            }
        }
    }

    // 초기 스태미나
    LeftHand.Stamina = MaxStamina;
    RightHand.Stamina = MaxStamina;
}

void ADownfallCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStamina(DeltaTime);
    UpdateClimbingState();

    // 착지 감지 및 Physics → Walking 모드 전환
    // Physics ON 중 OR Falling 중에 체크
    if (GetCapsuleComponent()->IsSimulatingPhysics() || 
        GetCharacterMovement()->MovementMode == MOVE_Falling)
    {
        // 바닥과의 충돌 체크
        FHitResult Hit;
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 10.0f); // 바닥 감지 거리
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        
        bool bHitGround = GetWorld()->SweepSingleByChannel(
            Hit,
            Start,
            End,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeSphere(GetCapsuleComponent()->GetScaledCapsuleRadius()),
            QueryParams
        );
        
        if (bHitGround)
        {
            // 경사 체크: 걸을 수 있는 경사인지
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(Hit.ImpactNormal.Z));
            float WalkableSlope = GetCharacterMovement()->GetWalkableFloorAngle();
            
            if (SlopeAngle <= WalkableSlope)
            {
                // 걸을 수 있는 경사 → Physics OFF, 운동량 초기화
                GetCapsuleComponent()->SetSimulatePhysics(false);
                GetCapsuleComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
                GetCapsuleComponent()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
                
                // Rotation 초기화 (중요!)
                FRotator CurrentRotation = GetActorRotation();
                FRotator UpRightRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f);
                SetActorRotation(UpRightRotation);
                
                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                
                UE_LOG(LogDownFall, Log, TEXT("Landed on walkable surface (%.1f°) - Physics disabled"), SlopeAngle);
            }
            else
            {
                UE_LOG(LogDownFall, Log, TEXT("Hit steep slope (%.1f°) - Physics continues"), SlopeAngle);
            }
        }
    }

#if !UE_BUILD_SHIPPING
    DrawDebugInfo();
#endif
}

void ADownfallCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IsValid(GrabLeftAction))
        {
            EIC->BindAction(GrabLeftAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnGrabLeftStarted);
            EIC->BindAction(GrabLeftAction, ETriggerEvent::Completed, this, &ADownfallCharacter::OnGrabLeftCompleted);
        }

        if (IsValid(GrabRightAction))
        {
            EIC->BindAction(GrabRightAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnGrabRightStarted);
            EIC->BindAction(GrabRightAction, ETriggerEvent::Completed, this, &ADownfallCharacter::OnGrabRightCompleted);
        }

        if (IsValid(LookAction))
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADownfallCharacter::OnLook);
        }
        
        if (IsValid(MoveAction))
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADownfallCharacter::OnMove);
        }
        
        if (IsValid(JumpAction))
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnJumpStarted);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADownfallCharacter::OnJumpCompleted);
        }
    }
}

// ========================================
// Input Handlers
// ========================================

void ADownfallCharacter::OnGrabLeftStarted(const FInputActionValue& Value)
{
    TryGrip(true);
}

void ADownfallCharacter::OnGrabLeftCompleted(const FInputActionValue& Value)
{
    ReleaseGrip(true);
}

void ADownfallCharacter::OnGrabRightStarted(const FInputActionValue& Value)
{
    TryGrip(false);
}

void ADownfallCharacter::OnGrabRightCompleted(const FInputActionValue& Value)
{
    ReleaseGrip(false);
}

void ADownfallCharacter::OnLook(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ADownfallCharacter::OnMove(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (!Controller) return;

    // 카메라(컨트롤러) 회전 기준으로 방향 계산
    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    
    // 등반 중: Physics Impulse 적용
    if (bIsClimbing)
    {
        UCapsuleComponent* Capsule = GetCapsuleComponent();
        if (!IsValid(Capsule)) return;

        const float ClimbSpeed = 500.0f; // 부드러운 속도 (조정 가능: 400-600)

        // 현재 잡고 있는 손의 벽면 Normal 가져오기
        FVector SurfaceNormal = FVector::UpVector;
        if (LeftHand.State == EHandState::Gripping)
        {
            SurfaceNormal = LeftHand.CurrentGrip.SurfaceNormal;
        }
        else if (RightHand.State == EHandState::Gripping)
        {
            SurfaceNormal = RightHand.CurrentGrip.SurfaceNormal;
        }

        // 벽면에 평행한 "위" 방향 계산
        FVector SurfaceUp = FVector::UpVector - SurfaceNormal * (FVector::UpVector | SurfaceNormal);
        SurfaceUp.Normalize();

        // 벽면에 평행한 "오른쪽" 방향 계산
        FVector SurfaceRight = FVector::CrossProduct(SurfaceNormal, SurfaceUp);
        SurfaceRight.Normalize();

        FVector TargetVelocity = FVector::ZeroVector;

        // 전후 이동 (W/S)
        if (MovementVector.Y != 0.0f)
        {
            TargetVelocity += SurfaceUp * MovementVector.Y * ClimbSpeed;
        }

        // 좌우 이동 (A/D)
        if (MovementVector.X != 0.0f)
        {
            TargetVelocity += SurfaceRight * MovementVector.X * ClimbSpeed;
        }

        // 현재 Velocity 가져오기
        FVector CurrentVelocity = Capsule->GetPhysicsLinearVelocity();
    
        // 부드러운 보간
        FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, GetWorld()->GetDeltaSeconds(), 5.0f);
    
        // Velocity 설정
        Capsule->SetPhysicsLinearVelocity(NewVelocity);
    }
    // 지상: 일반 이동
    else
    {
        // 전후 이동
        if (MovementVector.Y != 0.0f)
        {
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            AddMovementInput(Direction, MovementVector.Y);
        }

        // 좌우 이동
        if (MovementVector.X != 0.0f)
        {
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
            AddMovementInput(Direction, MovementVector.X);
        }
    }
}

void ADownfallCharacter::OnJumpStarted(const FInputActionValue& Value)
{
    // Grip 중이면 양손 놓고 점프
    if (bIsClimbing)
    {
        // 양손 모두 놓기
        if (LeftHand.State == EHandState::Gripping)
        {
            ReleaseGrip(true);
        }
        if (RightHand.State == EHandState::Gripping)
        {
            ReleaseGrip(false);
        }

        // 점프 임펄스 적용
        FVector JumpDirection = GetActorUpVector();
        GetCapsuleComponent()->AddImpulse(JumpDirection * 50000.0f, NAME_None, true);
        
        UE_LOG(LogDownFall, Log, TEXT("Jump from climbing"));
    }
    else
    {
        // 일반 점프 (Character 기본 기능)
        Jump();
    }
}

void ADownfallCharacter::OnJumpCompleted(const FInputActionValue& Value)
{
    // 일반 점프 종료
    StopJumping();
}

// Grip Logic
void ADownfallCharacter::TryGrip(bool bIsLeftHand)
{
    FHandData& Hand = bIsLeftHand ? LeftHand : RightHand;
    UPhysicsConstraintComponent* Constraint = bIsLeftHand ? LeftHandConstraint : RightHandConstraint;


    // 1. 상태 확인
    if (Hand.State == EHandState::Gripping)
    {
        return;
    }

    if (Hand.Stamina <= 0.0f)
    {
        UE_LOG(LogDownFall, Warning, TEXT("%s hand: No stamina"), bIsLeftHand ? TEXT("Left") : TEXT("Right"));
        return;
    }

    if (!IsValid(GripFinder) || !IsValid(FirstPersonCamera))
    {
        UE_LOG(LogDownFall, Error, TEXT("Missing components"));
        return;
    }
    
    // 2. 카메라 정보 가져오기
    FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
    FVector CameraForward = FirstPersonCamera->GetForwardVector();
    
    // 3. 그립 포인트 찾기
    FGripPointInfo GripInfo;
    bool bFound = GripFinder->FindGripPoint(CameraLocation, CameraForward, GripInfo);

    if (!bFound || !GripInfo.bIsValid)
    {
        UE_LOG(LogDownFall, Log, TEXT("%s hand: Grip failed"), bIsLeftHand ? TEXT("Left") : TEXT("Right"));
        return;
    }
    
    // 4. 물리 모드 전환 체크 (상태 업데이트 전)
    bool bWasBothHandsFree = AreBothHandsFree();

    // 5. Constraint 설정
    SetupConstraint(Constraint, GripInfo.WorldLocation);
    
    UE_LOG(LogDownFall, Log, TEXT("%s hand gripped: Angle=%.1f°, Quality=%.2f"), 
        bIsLeftHand ? TEXT("Left") : TEXT("Right"),
        GripInfo.SurfaceAngleDegrees, 
        GripInfo.GripQuality);

    // 6. 상태 업데이트
    Hand.State = EHandState::Gripping;
    Hand.CurrentGrip = GripInfo;
    Hand.GripStartTime = GetWorld()->GetTimeSeconds();

    // 7. 물리 모드 전환 (첫 Grip인 경우)
    if (bWasBothHandsFree)
    {
        GetCapsuleComponent()->SetSimulatePhysics(true);
        GetCharacterMovement()->SetMovementMode(MOVE_None);
        
        UE_LOG(LogDownFall, Log, TEXT("Physics mode activated"));
    }

    // 7. 이벤트 발생
    OnHandGripped(bIsLeftHand, GripInfo);

    UE_LOG(LogDownFall, Log, TEXT("%s hand gripped: Angle=%.1f°, Quality=%.2f"), 
        bIsLeftHand ? TEXT("Left") : TEXT("Right"), 
        GripInfo.SurfaceAngleDegrees, 
        GripInfo.GripQuality);
}

void ADownfallCharacter::ReleaseGrip(bool bIsLeftHand)
{
    FHandData& Hand = bIsLeftHand ? LeftHand : RightHand;
    UPhysicsConstraintComponent* Constraint = bIsLeftHand ? LeftHandConstraint : RightHandConstraint;

    if (Hand.State != EHandState::Gripping)
    {
        return;
    }

    // Constraint 해제
    BreakConstraint(Constraint);
    
    // 상태 업데이트
    Hand.State = EHandState::Free;
    Hand.CurrentGrip = FGripPointInfo();
    
    UE_LOG(LogDownFall, Log, TEXT("%s hand released"), bIsLeftHand ? TEXT("Left") : TEXT("Right"));
}

void ADownfallCharacter::SetupConstraint(UPhysicsConstraintComponent* Constraint, const FVector& TargetLocation)
{
    if (!IsValid(Constraint) || !IsValid(GetCapsuleComponent()))
    {
        return;
    }

    Constraint->SetWorldLocation(TargetLocation);
    
    Constraint->SetConstrainedComponents(
        GetCapsuleComponent(),
        NAME_None,
        nullptr, // World에 고정
        NAME_None
    );

    // ✅ Linear Constraint: 최대 거리 제한만 (팔 길이)
    Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, ArmLength);
    Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, ArmLength);
    Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, ArmLength);

    // Drive 제거
    Constraint->SetLinearPositionDrive(false, false, false);

    // Angular Constraint
    Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 45.0f);
    Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 45.0f);
    Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 30.0f);
}

void ADownfallCharacter::BreakConstraint(UPhysicsConstraintComponent* Constraint)
{
    if (IsValid(Constraint))
    {
        Constraint->BreakConstraint();
    }
}

// Stamina
void ADownfallCharacter::UpdateStamina(float DeltaTime)
{
    // 왼손
    if (LeftHand.State == EHandState::Gripping)
    {
        float DrainRate = GetStaminaDrainRate(LeftHand);
        LeftHand.Stamina = FMath::Max(0.0f, LeftHand.Stamina - DrainRate * DeltaTime);

        if (LeftHand.Stamina <= 0.0f)
        {
            UE_LOG(LogDownFall, Warning, TEXT("Left hand stamina depleted - releasing"));
            ReleaseGrip(true);
        }
    }
    else
    {
        LeftHand.Stamina = FMath::Min(MaxStamina, LeftHand.Stamina + StaminaRecoverPerSecond * DeltaTime);
    }

    // 오른손
    if (RightHand.State == EHandState::Gripping)
    {
        float DrainRate = GetStaminaDrainRate(RightHand);
        RightHand.Stamina = FMath::Max(0.0f, RightHand.Stamina - DrainRate * DeltaTime);

        if (RightHand.Stamina <= 0.0f)
        {
            UE_LOG(LogDownFall, Warning, TEXT("Right hand stamina depleted - releasing"));
            ReleaseGrip(false);
        }
    }
    else
    {
        RightHand.Stamina = FMath::Min(MaxStamina, RightHand.Stamina + StaminaRecoverPerSecond * DeltaTime);
    }
}

float ADownfallCharacter::GetStaminaDrainRate(const FHandData& Hand) const
{
    float BaseDrain = StaminaDrainPerSecond;

    // Phase 1: 단순 품질 기반 (Phase 3에서 곡선 매핑으로 교체 예정)
    if (Hand.CurrentGrip.GripQuality < 0.5f)
    {
        BaseDrain *= 2.0f;
    }

    return BaseDrain;
}

// State
void ADownfallCharacter::UpdateClimbingState()
{
    bool bWasClimbing = bIsClimbing;
    bIsClimbing = (LeftHand.State == EHandState::Gripping || RightHand.State == EHandState::Gripping);

    // 양손 모두 놓았을 때
    if (bWasClimbing && !bIsClimbing)
    {
        GetCapsuleComponent()->SetSimulatePhysics(false);
        
        // 바닥에 서있는지 체크
        FHitResult Hit;
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 100.0f); // 100cm 아래 체크 (Velocity 고려)
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        
        bool bHitGround = GetWorld()->SweepSingleByChannel(
            Hit,
            Start,
            End,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeSphere(GetCapsuleComponent()->GetScaledCapsuleRadius()),
            QueryParams
        );
        
        if (bHitGround)
        {
            // 경사 체크
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(Hit.ImpactNormal.Z));
            float WalkableSlope = GetCharacterMovement()->GetWalkableFloorAngle();
            
            if (SlopeAngle <= WalkableSlope)
            {
                // 걸을 수 있는 바닥 → 즉시 Walking 모드
                // Velocity 초기화
                GetCapsuleComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
                GetCapsuleComponent()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
                GetCharacterMovement()->Velocity = FVector::ZeroVector;
                
                // Rotation 초기화 (중요!)
                FRotator CurrentRotation = GetActorRotation();
                FRotator UpRightRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f); // Pitch, Roll = 0
                SetActorRotation(UpRightRotation);
                
                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                UE_LOG(LogDownFall, Log, TEXT("Released on ground - Walking mode"));
            }
            else
            {
                // 가파른 경사 → Falling
                GetCharacterMovement()->SetMovementMode(MOVE_Falling);
                UE_LOG(LogDownFall, Log, TEXT("Released on steep slope - Falling"));
            }
        }
        else
        {
            // 공중 → Falling
            GetCharacterMovement()->SetMovementMode(MOVE_Falling);
            UE_LOG(LogDownFall, Log, TEXT("Released in air - Falling"));
        }
    }
}

bool ADownfallCharacter::AreBothHandsFree() const
{
    return (LeftHand.State != EHandState::Gripping && RightHand.State != EHandState::Gripping);
}

// Debug
#if !UE_BUILD_SHIPPING
void ADownfallCharacter::DrawDebugInfo()
{
    if (!GEngine) return;

    int32 LineIndex = 1;
    
    // 등반 상태
    FString ClimbingStatus = bIsClimbing ? TEXT("Climbing") : TEXT("Not Climbing");
    GEngine->AddOnScreenDebugMessage(
        LineIndex++, 0.0f, FColor::Cyan,
        FString::Printf(TEXT("Status: %s"), *ClimbingStatus)
    );

    // Left Hand 정보
    if (LeftHand.State == EHandState::Gripping)
    {
        // 경사각
        float SlopeAngle = LeftHand.CurrentGrip.SurfaceAngleDegrees;
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Green,
            FString::Printf(TEXT("Left Slope: %.1f°"), SlopeAngle)
        );
        
        // 스태미나 소모량
        float DrainRate = GetStaminaDrainRate(LeftHand);
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Green,
            FString::Printf(TEXT("Left Drain: %.2f/s"), DrainRate)
        );
        
        // 현재 스태미나
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Green,
            FString::Printf(TEXT("Left Stamina: %.1f"), LeftHand.Stamina)
        );
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Red,
            TEXT("Left Hand: Free")
        );
    }

    // Right Hand 정보
    if (RightHand.State == EHandState::Gripping)
    {
        // 경사각
        float SlopeAngle = RightHand.CurrentGrip.SurfaceAngleDegrees;
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Blue,
            FString::Printf(TEXT("Right Slope: %.1f°"), SlopeAngle)
        );
        
        // 스태미나 소모량
        float DrainRate = GetStaminaDrainRate(RightHand);
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Blue,
            FString::Printf(TEXT("Right Drain: %.2f/s"), DrainRate)
        );
        
        // 현재 스태미나
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Blue,
            FString::Printf(TEXT("Right Stamina: %.1f"), RightHand.Stamina)
        );
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(
            LineIndex++, 0.0f, FColor::Red,
            TEXT("Right Hand: Free")
        );
    }

    // Physics 상태
    bool bPhysicsOn = GetCapsuleComponent()->IsSimulatingPhysics();
    FString PhysicsStatus = bPhysicsOn ? TEXT("Physics ON") : TEXT("Physics OFF");
    GEngine->AddOnScreenDebugMessage(
        LineIndex++, 0.0f, FColor::Yellow,
        FString::Printf(TEXT("Physics: %s"), *PhysicsStatus)
    );

    // Movement 모드
    FString MovementMode;
    switch (GetCharacterMovement()->MovementMode)
    {
        case MOVE_Walking: MovementMode = TEXT("Walking"); break;
        case MOVE_Falling: MovementMode = TEXT("Falling"); break;
        case MOVE_Flying: MovementMode = TEXT("Flying"); break;
        default: MovementMode = TEXT("Other"); break;
    }
    GEngine->AddOnScreenDebugMessage(
        LineIndex++, 0.0f, FColor::Yellow,
        FString::Printf(TEXT("Movement: %s"), *MovementMode)
    );
}
#endif