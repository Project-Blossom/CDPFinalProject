// File: Source/prototype/Private/DownfallCharacter.cpp
#include "DownfallCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Climbing/GripPointFinderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Monsters/FlyingPlatform.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Item/InventoryComponent.h"
#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "UI/AltitudeWidget.h"
#include "Blueprint/UserWidget.h"
#include "MountainGenWorldActor.h"
#include "MountainGenSettings.h"

DEFINE_LOG_CATEGORY(LogDownFall);

ADownfallCharacter::ADownfallCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Hand
    LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
    LeftHandMesh->SetupAttachment(FirstPersonCamera);
    LeftHandMesh->SetRelativeLocation(LeftHandRestPosition);
    LeftHandMesh->SetRelativeScale3D(FVector(0.3f)); // 작은 크기
    LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftHandMesh->SetCastShadow(false);

    RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
    RightHandMesh->SetupAttachment(FirstPersonCamera);
    RightHandMesh->SetRelativeLocation(RightHandRestPosition);
    RightHandMesh->SetRelativeScale3D(FVector(0.3f));
    RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightHandMesh->SetCastShadow(false);
    
    // Post Process Component (VFX용)
    PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComp"));
    PostProcessComp->SetupAttachment(FirstPersonCamera);
    PostProcessComp->bEnabled = true;
    PostProcessComp->bUnbound = false;
    PostProcessComp->BlendWeight = 1.0f;

    // AI Perception Stimuli Source (몬스터가 감지할 수 있게)
    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));

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

    // Inventory
    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void ADownfallCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogDownFall, Log, TEXT("DownfallCharacter BeginPlay called!"));

    // StimuliSource 등록
    if (StimuliSource)
    {
        StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
        StimuliSource->RegisterWithPerceptionSystem();
        UE_LOG(LogDownFall, Warning, TEXT("Player StimuliSource registered for Sight"));
    }
    else
    {
        UE_LOG(LogDownFall, Error, TEXT("Player StimuliSource is NULL! Creating at runtime..."));
        
        // 런타임에 생성 시도
        StimuliSource = NewObject<UAIPerceptionStimuliSourceComponent>(this, TEXT("StimuliSourceRuntime"));
        if (StimuliSource)
        {
            StimuliSource->RegisterComponent();
            StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
            StimuliSource->RegisterWithPerceptionSystem();
            UE_LOG(LogDownFall, Warning, TEXT("Player StimuliSource created at runtime"));
        }
    }

    // 초기 스태미나
    LeftHand.Stamina = MaxStamina;
    RightHand.Stamina = MaxStamina;

    // Attach Desaturation Material Instance 생성
    if (AttachDesaturationMaterial && PostProcessComp)
    {
        DesaturationMaterialInstance = UMaterialInstanceDynamic::Create(AttachDesaturationMaterial, this);
        if (DesaturationMaterialInstance)
        {
            DesaturationMaterialInstance->SetScalarParameterValue(FName("DesaturationAmount"), 0.0f);
            PostProcessComp->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, DesaturationMaterialInstance));
            UE_LOG(LogDownFall, Log, TEXT("Attach Desaturation Material Instance created"));
        }
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("AttachDesaturationMaterial not assigned - set in Blueprint"));
    }

    // Glitch Material Instance 생성
    if (GlitchMaterial && PostProcessComp)
    {
        GlitchMaterialInstance = UMaterialInstanceDynamic::Create(GlitchMaterial, this);
        if (GlitchMaterialInstance)
        {
            GlitchMaterialInstance->SetScalarParameterValue(FName("NoiseIntensity"), 0.0f);
            GlitchMaterialInstance->SetScalarParameterValue(FName("CurrentPattern"), 0.0f);
            PostProcessComp->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, GlitchMaterialInstance));
            
            // 첫 전환 시간 계산
            NextSwitchTime = CalculateNextSwitchInterval();
            
            UE_LOG(LogDownFall, Log, TEXT("Glitch Material Instance created"));
        }
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("GlitchMaterial not assigned - set in Blueprint"));
    }

    // Inventory
    if (Inventory && GripFinder)
    {
        Inventory->PlaceRangeCm = GripFinder->MaxReachDistance;
        Inventory->PlaceTraceDistanceCm = GripFinder->MaxReachDistance;
    }

    RefreshInventoryUIState();
    ApplyClimbingMappingContext();
    
    // Altitude Widget
    if (AltitudeWidgetClass)
    {
        AltitudeWidget = CreateWidget<UAltitudeWidget>(GetWorld(), AltitudeWidgetClass);
        if (AltitudeWidget)
        {
            AltitudeWidget->AddToViewport();
            UE_LOG(LogDownFall, Log, TEXT("Altitude Widget created and added to viewport"));
        }
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("AltitudeWidgetClass not assigned"));
    }
    
    InitialGroundHeight = GetGroundHeight();
    UE_LOG(LogDownFall, Log, TEXT("Initial ground height: %.2f"), InitialGroundHeight);
}

void ADownfallCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStamina(DeltaTime);
    UpdateInsanity(DeltaTime);
    UpdateClimbingState();
    CheckForPlatformAbduction();
    UpdateGlitchEffect();
    UpdateGlitchPatternSwitch(DeltaTime);
    UpdateAttachDesaturation(DeltaTime);
    UpdateAttachDesaturation(DeltaTime);
    UpdateHandPositions(DeltaTime);
    UpdateAltitudeUI();
    
    // AI Hearing: 의도적인 움직임만 소음 발생
    FVector Velocity = GetVelocity();
    float Speed = Velocity.Size();
    
    // 등반 중인지 확인 (멤버 변수 사용)
    // CRITICAL: 등반 중일 때는 더 높은 임계값 (200), 일반 이동은 100
    float NoiseThreshold = bIsClimbing ? 200.0f : 100.0f;
    
    if (Speed > NoiseThreshold)
    {
        // Noise 발생
        UAISense_Hearing::ReportNoiseEvent(
            GetWorld(),
            GetActorLocation(),
            1.0f,      // Loudness
            this,
            1000.0f,   // Max Range (10m)
            NAME_None
        );
        
        // 디버그 로그 (Verbose)
        UE_LOG(LogDownFall, Verbose, TEXT("Player making noise at speed: %.1f (threshold: %.1f)"), Speed, NoiseThreshold);
    }

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
            
            // Debug: Insanity Test
            if (DebugInsanityAction)
            {
                EIC->BindAction(DebugInsanityAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnDebugInsanity);
            }
        }

        if (IsValid(UseItemAction))
        {
            EIC->BindAction(UseItemAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnUseItemTriggered);
        }

        if (IsValid(ToggleInventoryAction))
        {
            EIC->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnToggleInventoryTriggered);
        }
    }
}

void ADownfallCharacter::OnUseItemTriggered(const FInputActionValue& Value)
{
    if (!Inventory)
    {
        return;
    }

    switch (ItemUseState)
    {
    case EItemUseState::None:
        return;

    case EItemUseState::InventoryOpen:
        TryPickHeldItemFromCursor();
        return;

    case EItemUseState::HoldingItem:
        TryUseHeldItem();
        return;

    case EItemUseState::PlacementPreview:
    {
        if (!Inventory->UseItem(HeldSlotIndex, this))
        {
            return;
        }

        Inventory->SetPreviewEnabled(false);

        const TArray<FItemStack>& Slots = Inventory->GetSlots();
        if (Slots.IsValidIndex(HeldSlotIndex) && Slots[HeldSlotIndex].IsValid())
        {
            ItemUseState = EItemUseState::HoldingItem;
        }
        else
        {
            HeldSlotIndex = INDEX_NONE;
            ItemUseState = EItemUseState::None;
        }

        RefreshInventoryUIState();
        return;
    }

    default:
        return;
    }
}

void ADownfallCharacter::OnToggleInventoryTriggered(const FInputActionValue& Value)
{
    if (!Inventory)
    {
        return;
    }

    switch (ItemUseState)
    {
    case EItemUseState::None:
        EnterInventoryFromCenter();
        break;

    case EItemUseState::InventoryOpen:
        CloseInventoryToEmptyHand();
        break;

    case EItemUseState::HoldingItem:
        EnterInventoryFromHeldSlot();
        break;

    case EItemUseState::PlacementPreview:
        Inventory->SetPreviewEnabled(false);
        ItemUseState = EItemUseState::HoldingItem;
        RefreshInventoryUIState();
        break;

    default:
        break;
    }
}

void ADownfallCharacter::ApplyClimbingMappingContext()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsys =
                LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (ClimbingMappingContext)
                {
                    Subsys->ClearAllMappings();
                    Subsys->AddMappingContext(ClimbingMappingContext, 0);
                }
            }
        }
    }
}

// Input Handlers
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

    if (ItemUseState == EItemUseState::InventoryOpen)
    {
        TryMoveInventoryCursorFromInput(MovementVector);
        return;
    }

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
    // 등반 중이면 양손 놓고 점프
    if (bIsClimbing)
    {
        UE_LOG(LogDownFall, Log, TEXT("Jump from climbing - releasing grips"));
        
        // 양손 모두 놓기
        if (LeftHand.State == EHandState::Gripping)
        {
            ReleaseGrip(true);
        }
        if (RightHand.State == EHandState::Gripping)
        {
            ReleaseGrip(false);
        }

        // Physics OFF (ReleaseGrip에서 이미 했지만 명시적으로)
        GetCapsuleComponent()->SetSimulatePhysics(false);
        
        // 카메라 반대 방향으로 밀기 (벽에서 멀어지기)
        FVector PushDirection = FVector::ZeroVector;
        
        if (FirstPersonCamera)
        {
            FVector CameraForward = FirstPersonCamera->GetForwardVector();
            PushDirection = CameraForward; // 카메라 방향 (벽 쪽으로)
            PushDirection.Z = 0; // 수평으로만
            PushDirection.Normalize();
        }
        
        // 밀어내기 (수평) + 점프 (수직)
        FVector JumpVelocity = PushDirection * 300.0f + FVector::UpVector * 400.0f;
        
        // Velocity 직접 설정
        GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        GetCharacterMovement()->Velocity = JumpVelocity;
        
        UE_LOG(LogDownFall, Log, TEXT("Jump velocity: %s"), *JumpVelocity.ToString());
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

// Debug: Insanity Test
void ADownfallCharacter::OnDebugInsanity(const FInputActionValue& Value)
{
    AddInsanity(10.0f);
    UE_LOG(LogDownFall, Warning, TEXT("Debug: Added 10 Insanity (Test key pressed)"));
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
    
    // 5. 상태 업데이트
    Hand.State = EHandState::Gripping;
    Hand.CurrentGrip = GripInfo;
    
    // 6. FlyingPlatform 체크 (Constraint 설정 전!)
    bool bGrippedMonster = false;
    
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->SweepMultiByChannel(
        Hits,
        GripInfo.WorldLocation,
        GripInfo.WorldLocation + FVector(0, 0, 1),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(300.0f),  // 몬스터 잡기 판정 범위 (3m)
        Params
    );

    for (const FHitResult& Hit : Hits)
    {
        if (Hit.GetActor() && Hit.GetActor()->Implements<UClimbableSurface>())
        {
            Hand.GrippedActor = Hit.GetActor();
        
            if (AFlyingPlatform* Platform = Cast<AFlyingPlatform>(Hit.GetActor()))
            {
                Platform->OnPlayerGrab(this);
                UE_LOG(LogDownFall, Log, TEXT("Grabbed Flying Platform: %s"), *Platform->GetName());
                
                // 동적 Constraint 설정
                SetupConstraintToActor(Constraint, Platform, GripInfo.WorldLocation);
                bGrippedMonster = true;
            }
            break;
        }
    }
    
    // 7. 일반 지형 Constraint 설정 (몬스터가 아닌 경우만)
    if (!bGrippedMonster)
    {
        SetupConstraint(Constraint, GripInfo.WorldLocation);
    }
    
    Hand.GripStartTime = GetWorld()->GetTimeSeconds();

    // 8. 물리 모드 전환 (첫 Grip인 경우)
    if (bWasBothHandsFree)
    {
        GetCapsuleComponent()->SetSimulatePhysics(true);
        GetCharacterMovement()->SetMovementMode(MOVE_None);
        
        UE_LOG(LogDownFall, Log, TEXT("Physics mode activated"));
    }

    // 9. 이벤트 발생
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

    if (Hand.GrippedActor)
    {
        if (AFlyingPlatform* Platform = Cast<AFlyingPlatform>(Hand.GrippedActor))
        {
            Platform->OnPlayerRelease();
            UE_LOG(LogDownFall, Log, TEXT("Released Flying Platform: %s"), *Platform->GetName());
        }
        Hand.GrippedActor = nullptr;
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

void ADownfallCharacter::SetupConstraintToActor(UPhysicsConstraintComponent* Constraint, AActor* TargetActor, const FVector& GripLocation)
{
    if (!Constraint || !TargetActor) return;

    // 타겟의 Primitive Component 찾기
    UPrimitiveComponent* TargetComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
    if (!TargetComp)
    {
        TargetComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    }

    if (!TargetComp)
    {
        UE_LOG(LogDownFall, Error, TEXT("Target actor has no primitive component"));
        return;
    }

    // Constraint 연결
    Constraint->SetConstrainedComponents(
        GetCapsuleComponent(),
        NAME_None,
        TargetComp,
        NAME_None
    );

    // 로컬 오프셋 계산 (타겟의 로컬 좌표로 변환)
    FVector LocalOffset = TargetActor->GetActorTransform().InverseTransformPosition(GripLocation);
    Constraint->SetConstraintReferencePosition(EConstraintFrame::Frame2, LocalOffset);

    // Linear Limits
    Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, 100.0f);
    Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, 100.0f);
    Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, 100.0f);

    // Angular Limits
    Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 45.0f);
    Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 45.0f);
    Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 30.0f);

    UE_LOG(LogDownFall, Log, TEXT("Constraint set to dynamic actor: %s"), *TargetActor->GetName());
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

void ADownfallCharacter::AddInsanity(float Amount)
{
    Insanity = FMath::Clamp(Insanity + Amount, 0.0f, MaxInsanity);
    UpdateInsanityEffects();
    
    UE_LOG(LogDownFall, Log, TEXT("Insanity increased by %.1f, now: %.1f"), Amount, Insanity);
}

void ADownfallCharacter::UpdateInsanity(float DeltaTime)
{
    if (Insanity > 0.0f)
    {
        // 혼란 상태일 때 (70 이상)
        if (Insanity >= InsanityThreshold)
        {
            // 자가 증폭: 초당 0.1씩 증가
            Insanity = FMath::Min(MaxInsanity, Insanity + InsanityGrowthRate * DeltaTime);
        }
        else
        {
            // 정상 상태: 초당 0.1씩 감소
            Insanity = FMath::Max(0.0f, Insanity - InsanityDecayRate * DeltaTime);
        }
        
        UpdateInsanityEffects();
    }
}

void ADownfallCharacter::UpdateInsanityEffects()
{
    bool bShouldBeConfused = Insanity >= InsanityThreshold;
    
    if (bShouldBeConfused != bIsConfused)
    {
        bIsConfused = bShouldBeConfused;
        
        if (bIsConfused)
        {
            UE_LOG(LogDownFall, Warning, TEXT("Insanity confusion effect enabled! (%.1f >= %.1f)"), 
                Insanity, InsanityThreshold);
        }
        else
        {
            UE_LOG(LogDownFall, Log, TEXT("Insanity confusion effect disabled"));
        }
    }
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
        
        // Rotation 초기화
        FRotator CurrentRotation = GetActorRotation();
        FRotator UpRightRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f);
        SetActorRotation(UpRightRotation);
        
        // 바닥에 서있는지 체크
        FHitResult Hit;
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 100.0f); // 100cm 아래 체크
        
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

// Platform Abduction (납치)
void ADownfallCharacter::CheckForPlatformAbduction()
{
    // 양손 중 하나라도 Gripping 상태인지 확인
    if (LeftHand.State != EHandState::Gripping && RightHand.State != EHandState::Gripping)
    {
        return;  // 아무것도 안 잡고 있음
    }

    // 플레이어 위치
    FVector PlayerLocation = GetActorLocation();

    // Sweep으로 근처 FlyingPlatform 찾기
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->SweepMultiByChannel(
        Hits,
        PlayerLocation,
        PlayerLocation + FVector(0, 0, 1),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(300.0f),  // 3m 범위
        Params
    );

    for (const FHitResult& Hit : Hits)
    {
        if (!Hit.GetActor() || !Hit.GetActor()->Implements<UClimbableSurface>())
            continue;

        AFlyingPlatform* Platform = Cast<AFlyingPlatform>(Hit.GetActor());
        if (!Platform)
            continue;

        // 양손 체크: 이미 이 Platform을 잡고 있으면 스킵
        if (LeftHand.GrippedActor == Platform || RightHand.GrippedActor == Platform)
            continue;

        // 납치! 한 손이라도 비어있으면 그 손으로 잡음
        bool bAbductedLeft = false;
        bool bAbductedRight = false;

        if (LeftHand.State == EHandState::Gripping && LeftHand.GrippedActor != Platform)
        {
            // 왼손으로 지형 잡고 있음 → 납치!
            AbductByPlatform(true, Platform);
            bAbductedLeft = true;
        }

        if (RightHand.State == EHandState::Gripping && RightHand.GrippedActor != Platform)
        {
            // 오른손으로 지형 잡고 있음 → 납치!
            AbductByPlatform(false, Platform);
            bAbductedRight = true;
        }

        if (bAbductedLeft || bAbductedRight)
        {
            UE_LOG(LogDownFall, Warning, TEXT("ABDUCTED by %s!"), *Platform->GetName());
            break;
        }
    }
}

void ADownfallCharacter::AbductByPlatform(bool bIsLeftHand, AFlyingPlatform* Platform)
{
    if (!Platform) return;

    FHandData& Hand = bIsLeftHand ? LeftHand : RightHand;
    UPhysicsConstraintComponent* Constraint = bIsLeftHand ? LeftHandConstraint : RightHandConstraint;

    // 1. 기존 Grip 정보 업데이트
    FVector PlatformGrabLocation = Platform->GetGrabLocation();
    Hand.CurrentGrip.WorldLocation = PlatformGrabLocation;
    Hand.GrippedActor = Platform;

    // 2. Constraint 재설정 (Platform 중앙으로)
    SetupConstraintToActor(Constraint, Platform, PlatformGrabLocation);

    // 3. Platform에게 알림
    Platform->OnPlayerGrab(this);

    UE_LOG(LogDownFall, Warning, TEXT("%s hand abducted by Platform: %s"), 
        bIsLeftHand ? TEXT("Left") : TEXT("Right"), *Platform->GetName());
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

    // Insanity 표시
    FColor InsanityColor = bIsConfused ? FColor::Red : FColor::Magenta;
    GEngine->AddOnScreenDebugMessage(
        LineIndex++, 0.0f, InsanityColor,
        FString::Printf(TEXT("Insanity: %.1f / %.1f %s"), 
            Insanity, MaxInsanity, bIsConfused ? TEXT("[CONFUSED]") : TEXT(""))
    );
}
#endif

void ADownfallCharacter::UpdateGlitchEffect()
{
    if (!GlitchMaterialInstance)
        return;
    
    // Insanity에 따라 NoiseIntensity 조절 (점진적, 0~100 → 0~1)
    float TargetIntensity = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 100.0f),  // Insanity 범위
        FVector2D(0.0f, 1.0f),     // NoiseIntensity 범위
        Insanity
    );
    
    // 부드러운 전환
    CurrentNoiseIntensity = FMath::FInterpTo(
        CurrentNoiseIntensity, 
        TargetIntensity, 
        GetWorld()->GetDeltaSeconds(), 
        2.0f
    );
    
    // Material Parameter 업데이트
    GlitchMaterialInstance->SetScalarParameterValue(
        FName("NoiseIntensity"), 
        CurrentNoiseIntensity
    );
    
#if !UE_BUILD_SHIPPING
    // 디버그 표시
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            1, 
            0.0f, 
            FColor::Red,
            FString::Printf(TEXT("Glitch: %.0f%% (Insanity: %.1f, Pattern: %d)"), 
                CurrentNoiseIntensity * 100.0f, Insanity, CurrentPattern)
        );
    }
#endif
}

void ADownfallCharacter::ShowAttachDesaturation(float Amount)
{
    if (!DesaturationMaterialInstance)
    {
        UE_LOG(LogDownFall, Warning, TEXT("ShowAttachDesaturation: DesaturationMaterialInstance is NULL"));
        return;
    }

    bShowingDesaturation = true;
    DesaturationAmount = Amount;
    
    UE_LOG(LogDownFall, Log, TEXT("ShowAttachDesaturation: Amount=%.2f"), Amount);
}

void ADownfallCharacter::HideAttachDesaturation()
{
    bShowingDesaturation = false;
    
    UE_LOG(LogDownFall, Log, TEXT("HideAttachDesaturation called"));
}

void ADownfallCharacter::UpdateAttachDesaturation(float DeltaTime)
{
    if (!DesaturationMaterialInstance)
        return;

    float TargetAmount = bShowingDesaturation ? DesaturationAmount : 0.0f;

    // 부드러운 페이드 인/아웃
    CurrentDesaturationAmount = FMath::FInterpTo(
        CurrentDesaturationAmount,
        TargetAmount,
        DeltaTime,
        DesaturationFadeSpeed
    );

    // Material Parameter 업데이트
    DesaturationMaterialInstance->SetScalarParameterValue(FName("DesaturationAmount"), CurrentDesaturationAmount);
}

void ADownfallCharacter::UpdateGlitchPatternSwitch(float DeltaTime)
{
    if (!GlitchMaterialInstance)
        return;
    
    TimeSinceLastSwitch += DeltaTime;
    
    if (TimeSinceLastSwitch >= NextSwitchTime)
    {
        // 패턴 전환 (0 → 1 → 2 → 0)
        CurrentPattern = (CurrentPattern + 1) % 3;
        
        GlitchMaterialInstance->SetScalarParameterValue(
            FName("CurrentPattern"), 
            (float)CurrentPattern
        );
        
        // 다음 전환 시간 계산
        TimeSinceLastSwitch = 0.0f;
        NextSwitchTime = CalculateNextSwitchInterval();
        
        UE_LOG(LogDownFall, Log, TEXT("Glitch Pattern switched to %d, next switch in %.2f seconds"), 
            CurrentPattern, NextSwitchTime);
    }
}

float ADownfallCharacter::CalculateNextSwitchInterval() const
{
    // Insanity가 높을수록 전환 빈도 증가 (0~100 → 1~5배)
    float FrequencyMultiplier = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 100.0f),   // Insanity 범위
        FVector2D(1.0f, 5.0f),     // 전환 빈도 배율
        Insanity
    );
    
    // 기본 간격을 빈도로 나눔
    float BaseInterval = PatternSwitchBaseInterval / FrequencyMultiplier;
    
    // 랜덤 변동 추가
    float RandomVariation = FMath::RandRange(-PatternSwitchRandomness, PatternSwitchRandomness);
    
    // 최소 0.1초는 유지
    return FMath::Max(0.1f, BaseInterval + RandomVariation);
}

void ADownfallCharacter::RefreshInventoryUIState()
{
    const bool bInventoryOpen = (ItemUseState == EItemUseState::InventoryOpen);
    const bool bPreviewing = (ItemUseState == EItemUseState::PlacementPreview);

    BP_UpdateInventoryMode(bInventoryOpen, InventoryCursorIndex, HeldSlotIndex, bPreviewing);
}

void ADownfallCharacter::MoveInventoryCursor(int32 DX, int32 DY)
{
    int32 X = InventoryCursorIndex % 5;
    int32 Y = InventoryCursorIndex / 5;

    X = FMath::Clamp(X + DX, 0, 4);
    Y = FMath::Clamp(Y + DY, 0, 4);

    InventoryCursorIndex = Y * 5 + X;
    RefreshInventoryUIState();
}

bool ADownfallCharacter::TryMoveInventoryCursorFromInput(const FVector2D& MovementVector)
{
    const float AbsX = FMath::Abs(MovementVector.X);
    const float AbsY = FMath::Abs(MovementVector.Y);

    FVector2D DesiredDir = FVector2D::ZeroVector;

    if (AbsX < 0.5f && AbsY < 0.5f)
    {
        bCursorInputHeld = false;
        LastCursorInputDir = FVector2D::ZeroVector;
        return false;
    }

    if (AbsX >= AbsY)
    {
        DesiredDir.X = (MovementVector.X > 0.0f) ? 1.0f : -1.0f;
    }
    else
    {
        // 위/아래가 반대로 느껴지면 여기만 바꾸면 됨
        DesiredDir.Y = (MovementVector.Y > 0.0f) ? -1.0f : 1.0f;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    if (!bCursorInputHeld || DesiredDir != LastCursorInputDir)
    {
        bCursorInputHeld = true;
        LastCursorInputDir = DesiredDir;
        NextCursorRepeatTime = Now + CursorInitialRepeatDelay;

        MoveInventoryCursor((int32)DesiredDir.X, (int32)DesiredDir.Y);
        return true;
    }

    if (Now >= NextCursorRepeatTime)
    {
        NextCursorRepeatTime = Now + CursorRepeatInterval;
        MoveInventoryCursor((int32)DesiredDir.X, (int32)DesiredDir.Y);
        return true;
    }

    return false;
}

void ADownfallCharacter::EnterInventoryFromCenter()
{
    ItemUseState = EItemUseState::InventoryOpen;
    InventoryCursorIndex = 12;
    HeldSlotIndex = INDEX_NONE;

    if (Inventory)
    {
        Inventory->SetPreviewEnabled(false);
    }

    RefreshInventoryUIState();
}

void ADownfallCharacter::EnterInventoryFromHeldSlot()
{
    ItemUseState = EItemUseState::InventoryOpen;

    if (IsValidInventorySlotIndex(HeldSlotIndex))
    {
        InventoryCursorIndex = HeldSlotIndex;
    }
    else
    {
        InventoryCursorIndex = 12;
    }

    if (Inventory)
    {
        Inventory->SetPreviewEnabled(false);
    }

    HeldSlotIndex = INDEX_NONE;
    RefreshInventoryUIState();
}

void ADownfallCharacter::CloseInventoryToEmptyHand()
{
    ItemUseState = EItemUseState::None;
    InventoryCursorIndex = 12;
    HeldSlotIndex = INDEX_NONE;

    if (Inventory)
    {
        Inventory->SetPreviewEnabled(false);
    }

    RefreshInventoryUIState();
}

bool ADownfallCharacter::TryPickHeldItemFromCursor()
{
    if (!Inventory)
    {
        return false;
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();

    if (!Slots.IsValidIndex(InventoryCursorIndex))
    {
        return false;
    }

    if (!Slots[InventoryCursorIndex].IsValid())
    {
        return false;
    }

    HeldSlotIndex = InventoryCursorIndex;
    ItemUseState = EItemUseState::HoldingItem;
    RefreshInventoryUIState();
    return true;
}

bool ADownfallCharacter::TryUseHeldItem()
{
    if (!Inventory || !IsValidInventorySlotIndex(HeldSlotIndex))
    {
        return false;
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();

    if (!Slots.IsValidIndex(HeldSlotIndex) || !Slots[HeldSlotIndex].IsValid())
    {
        HeldSlotIndex = INDEX_NONE;
        ItemUseState = EItemUseState::None;
        RefreshInventoryUIState();
        return false;
    }

    if (IsPlaceableSlot(HeldSlotIndex))
    {
        Inventory->SetPreviewSlotIndex(HeldSlotIndex);
        Inventory->SetPreviewEnabled(true);
        ItemUseState = EItemUseState::PlacementPreview;
        RefreshInventoryUIState();
        return true;
    }

    const bool bUsed = Inventory->UseItem(HeldSlotIndex, this);
    if (!bUsed)
    {
        return false;
    }

    const TArray<FItemStack>& AfterSlots = Inventory->GetSlots();
    if (!AfterSlots.IsValidIndex(HeldSlotIndex) || !AfterSlots[HeldSlotIndex].IsValid())
    {
        HeldSlotIndex = INDEX_NONE;
        ItemUseState = EItemUseState::None;
    }
    else
    {
        ItemUseState = EItemUseState::HoldingItem;
    }

    RefreshInventoryUIState();
    return true;
}

bool ADownfallCharacter::IsValidInventorySlotIndex(int32 Index) const
{
    return Inventory && Inventory->GetSlots().IsValidIndex(Index);
}

bool ADownfallCharacter::IsPlaceableSlot(int32 Index) const
{
    if (!Inventory || !IsValidInventorySlotIndex(Index))
    {
        return false;
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();
    const FItemStack& Stack = Slots[Index];
    if (!Stack.IsValid())
    {
        return false;
    }

    UWorld* W = GetWorld();
    UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
    UItemSubsystem* IS = GI ? GI->GetSubsystem<UItemSubsystem>() : nullptr;
    if (!IS)
    {
        return false;
    }

    const UItemDefinition* Def = IS->GetItemDefinitionById(Stack.ItemId);
    if (!Def)
    {
        return false;
    }

    return (Def->UseType == EItemUseType::PlaceActor && Def->PlaceActorClass != nullptr);
}

void ADownfallCharacter::UpdateHandPositions(float DeltaTime)
{
    if (!LeftHandMesh || !RightHandMesh)
        return;

    // 왼손 목표 위치
    FVector LeftTarget = GetHandTargetPosition(true);
    FVector CurrentLeftPos = LeftHandMesh->GetRelativeLocation();
    FVector NewLeftPos = FMath::VInterpTo(CurrentLeftPos, LeftTarget, DeltaTime, HandMoveSpeed);
    LeftHandMesh->SetRelativeLocation(NewLeftPos);

    // 오른손 목표 위치
    FVector RightTarget = GetHandTargetPosition(false);
    FVector CurrentRightPos = RightHandMesh->GetRelativeLocation();
    FVector NewRightPos = FMath::VInterpTo(CurrentRightPos, RightTarget, DeltaTime, HandMoveSpeed);
    RightHandMesh->SetRelativeLocation(NewRightPos);
}

FVector ADownfallCharacter::GetHandTargetPosition(bool bIsLeftHand) const
{
    const FHandData& Hand = bIsLeftHand ? LeftHand : RightHand;
    
    // 손이 잡고 있으면
    if (Hand.State == EHandState::Gripping && Hand.CurrentGrip.bIsValid)
    {
        // 월드 공간의 Grip 위치를 카메라 로컬 공간으로 변환
        FVector GripWorldLocation = Hand.CurrentGrip.WorldLocation;
        
        if (FirstPersonCamera)
        {
            FTransform CameraTransform = FirstPersonCamera->GetComponentTransform();
            FVector LocalGripPos = CameraTransform.InverseTransformPosition(GripWorldLocation);
            
            return LocalGripPos;
        }
    }
    
    // 손을 놓았으면 휴식 위치로
    return bIsLeftHand ? LeftHandRestPosition : RightHandRestPosition;
}

void ADownfallCharacter::UpdateAltitudeUI()
{
    if (!AltitudeWidget)
        return;

    float CurrentZ = GetActorLocation().Z;
    float MaxHeight = 5000.0f; // 기본값 500m
    
    AMountainGenWorldActor* MountainActor = FindMountainGenActor();
    if (MountainActor)
    {
        MaxHeight = MountainActor->Settings.CliffHeightCm;
    }
    
    AltitudeWidget->UpdateAltitude(CurrentZ, InitialGroundHeight, MaxHeight);
}

AMountainGenWorldActor* ADownfallCharacter::FindMountainGenActor()
{
    // 캐싱 (매 프레임 찾지 않기)
    static AMountainGenWorldActor* CachedActor = nullptr;
    
    if (CachedActor)
        return CachedActor;
    
    // 월드에서 찾기
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMountainGenWorldActor::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        CachedActor = Cast<AMountainGenWorldActor>(FoundActors[0]);
        UE_LOG(LogDownFall, Log, TEXT("MountainGenWorldActor found: %s"), *CachedActor->GetName());
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("MountainGenWorldActor not found in level"));
    }
    
    return CachedActor;
}

float ADownfallCharacter::GetGroundHeight() const
{
    // LineTrace로 지면 찾기
    FHitResult Hit;
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0, 0, 100000.0f); // 1000m 아래까지
    
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_WorldStatic,
        Params
    );
    
    if (bHit)
    {
        return Hit.Location.Z;
    }
    
    // 못 찾으면 0
    return 0.0f;
}