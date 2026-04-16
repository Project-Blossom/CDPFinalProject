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
#include "UI/PauseMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "MountainGenWorldActor.h"
#include "MountainGenSettings.h"
#include "Core/DownfallGameMode.h"
#include "TimerManager.h"
#include <limits>
#include "Components/SceneComponent.h"
#include "UObject/UnrealType.h"

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
    PostProcessComp->Settings.bOverride_SceneFringeIntensity = true;
    PostProcessComp->Settings.SceneFringeIntensity = BaseChromaticAberration;
    PostProcessComp->Settings.bOverride_FilmGrainIntensity = true;
    PostProcessComp->Settings.FilmGrainIntensity = BaseFilmGrainIntensity;
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

    SafetyLineConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SafetyLineConstraint"));
    SafetyLineConstraint->SetupAttachment(GetCapsuleComponent());
    SafetyLineConstraint->SetDisableCollision(true);

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

    // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Log, TEXT("DownfallCharacter BeginPlay called!"));

    // StimuliSource 등록
    if (StimuliSource)
    {
        StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
        StimuliSource->RegisterWithPerceptionSystem();
        // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Warning, TEXT("Player StimuliSource registered for Sight"));
    }
    else
    {
        // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Error, TEXT("Player StimuliSource is NULL! Creating at runtime..."));

        // 런타임에 생성 시도
        StimuliSource = NewObject<UAIPerceptionStimuliSourceComponent>(this, TEXT("StimuliSourceRuntime"));
        if (StimuliSource)
        {
            StimuliSource->RegisterComponent();
            StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
            StimuliSource->RegisterWithPerceptionSystem();
            // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Warning, TEXT("Player StimuliSource created at runtime"));
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

    // Dirt Mask Material Instance 생성
    if (DirtMaskMaterial && PostProcessComp)
    {
        DirtMaskMaterialInstance = UMaterialInstanceDynamic::Create(DirtMaskMaterial, this);
        if (DirtMaskMaterialInstance)
        {
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("DirtIntensity"), DirtIntensity);
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("BlurOffset"), DirtBlurOffset);
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("TintStrength"), DirtTintStrength);
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("LightResponse"), DirtLightResponse);
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("LightThreshold"), DirtLightThreshold);
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("LightSoftness"), DirtLightSoftness);
            DirtMaskMaterialInstance->SetScalarParameterValue(FName("DirtExposure"), DirtExposure);
            PostProcessComp->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, DirtMaskMaterialInstance));

            UE_LOG(LogDownFall, Log, TEXT("Dirt Mask Material Instance created"));
        }
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("DirtMaskMaterial not assigned - set in Blueprint"));
    }

    // Edge Blur Material Instance 생성
    if (EdgeBlurMaterial && PostProcessComp)
    {
        EdgeBlurMaterialInstance = UMaterialInstanceDynamic::Create(EdgeBlurMaterial, this);
        if (EdgeBlurMaterialInstance)
        {
            EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurStart"), BlurStart);
            EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurEnd"), BlurEnd);
            EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurStrength"), BlurStrength);
            EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurOffset"), BlurOffset);
            PostProcessComp->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, EdgeBlurMaterialInstance));

            UE_LOG(LogDownFall, Log, TEXT("Edge Blur Material Instance created"));
        }
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("EdgeBlurMaterial not assigned - set in Blueprint"));
    }

    // Lens Distortion Material 초기화
    if (LensDistortionMaterial)
    {
        LensDistortionMaterialInstance = UMaterialInstanceDynamic::Create(LensDistortionMaterial, this);
        if (LensDistortionMaterialInstance)
        {
            LensDistortionMaterialInstance->SetScalarParameterValue(FName("K1"), BaseK1);
            LensDistortionMaterialInstance->SetScalarParameterValue(FName("K2"), K2);
            PostProcessComp->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, LensDistortionMaterialInstance));

            UE_LOG(LogTemp, Log, TEXT("Lens Distortion Material initialized"));
        }
    }
    else
    {
        UE_LOG(LogDownFall, Warning, TEXT("Lens Distortion Material not assigned - set in Blueprint"));
    }

    if (VignetteMaterial)
    {
        VignetteMaterialInstance = UMaterialInstanceDynamic::Create(VignetteMaterial, this);
        if (VignetteMaterialInstance)
        {
            VignetteMaterialInstance->SetScalarParameterValue(FName("GradientStart"), VignetteGradientStart);
            VignetteMaterialInstance->SetScalarParameterValue(FName("GradientEnd"), VignetteGradientEnd);
            VignetteMaterialInstance->SetVectorParameterValue(FName("VignetteOffset"), FLinearColor(0, 0, 0));
            PostProcessComp->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, VignetteMaterialInstance));

            UE_LOG(LogTemp, Log, TEXT("Vignette Material initialized"));
        }
    }

    if (LeftHandMesh)
    {
        LeftHandMaterialInstance = LeftHandMesh->CreateDynamicMaterialInstance(0);
        if (LeftHandMaterialInstance)
        {
            UE_LOG(LogDownFall, Log, TEXT("Left Hand Material Instance created"));
        }
    }

    if (RightHandMesh)
    {
        RightHandMaterialInstance = RightHandMesh->CreateDynamicMaterialInstance(0);
        if (RightHandMaterialInstance)
        {
            UE_LOG(LogDownFall, Log, TEXT("Right Hand Material Instance created"));
        }
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

    CachedMountainActor = FindMountainGenActor();

    ApplyDirtMaskParameters(true);
    ApplyEdgeBlurParameters(true);
    UpdateAltitudeUI();

    InitialGroundHeight = GetGroundHeight();
    UE_LOG(LogDownFall, Log, TEXT("Initial ground height: %.2f"), InitialGroundHeight);

    StartLowFrequencyUpdatesIfNeeded();
}

void ADownfallCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 앵커가 한 손이라도 걸려 있으면 보조 정보 갱신
    if (IsAnchorGripActive())
    {
        UpdateAnchorGrip(DeltaTime);
    }

    UpdateStamina(DeltaTime);
    UpdateInsanity(DeltaTime);
    UpdateClimbingState();
    UpdateSafetyLine(DeltaTime);
    UpdateGlitchEffect();
    UpdateGlitchPatternSwitch(DeltaTime);
    UpdateAttachDesaturation(DeltaTime);
    UpdateLensDistortionEffect();
    UpdateVignetteEffect(DeltaTime);
    UpdateHandPositions(DeltaTime);
    UpdateHandStaminaVisuals(DeltaTime);

    ApplyDirtMaskParameters(false);
    ApplyEdgeBlurParameters(false);

    StartLowFrequencyUpdatesIfNeeded();
    StopLowFrequencyUpdatesIfPossible();

    if (bDebugFlyMode)
    {
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            float VerticalInput = 0.0f;

            if (PC->IsInputKeyDown(EKeys::SpaceBar))
            {
                VerticalInput += 1.0f;
            }
            if (PC->IsInputKeyDown(EKeys::LeftControl) || PC->IsInputKeyDown(EKeys::RightControl))
            {
                VerticalInput -= 1.0f;
            }

            if (!FMath::IsNearlyZero(VerticalInput))
            {
                AddMovementInput(FVector::UpVector, VerticalInput * (DebugFlyVerticalSpeed / FMath::Max(1.0f, DebugFlySpeed)));
            }
        }
    }

    // AI Hearing: 의도적인 움직임만 소음 발생
    FVector Velocity = GetVelocity();
    float Speed = Velocity.Size();

    // 등반 중인지 확인 (멤버 변수 사용)
    // CRITICAL: 등반 중일 때는 더 높은 임계값 (200), 일반 이동은 100
    float NoiseThreshold = bIsClimbing ? 200.0f : 100.0f;

    if (Speed > NoiseThreshold)
    {
        UAISense_Hearing::ReportNoiseEvent(
            GetWorld(),
            GetActorLocation(),
            1.0f,
            this,
            1000.0f,
            NAME_None
        );

        // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Verbose, TEXT("Player making noise at speed: %.1f (threshold: %.1f)"), Speed, NoiseThreshold);
    }

    // 착지 감지 및 Physics → Walking 모드 전환
    // 등반이 완전히 끝난 상태에서만 체크
    if (!bIsClimbing &&
        (GetCapsuleComponent()->IsSimulatingPhysics() ||
            GetCharacterMovement()->MovementMode == MOVE_Falling))
    {
        FHitResult Hit;
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 10.0f);

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
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(Hit.ImpactNormal.Z));
            float WalkableSlope = GetCharacterMovement()->GetWalkableFloorAngle();

            if (SlopeAngle <= WalkableSlope)
            {
                GetCapsuleComponent()->SetSimulatePhysics(false);
                GetCapsuleComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
                GetCapsuleComponent()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

                FRotator CurrentRotation = GetActorRotation();
                FRotator UpRightRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f);
                SetActorRotation(UpRightRotation);

                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                DisengageSafetyLineConstraint();

                // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Log, TEXT("Landed on walkable surface (%.1f°) - Physics disabled"), SlopeAngle);
            }
            else
            {
                // [DISABLED FOR DEMO] UE_LOG(LogDownFall, Log, TEXT("Hit steep slope (%.1f°) - Physics continues"), SlopeAngle);
            }
        }
    }

#if !UE_BUILD_SHIPPING
    if (CachedMountainActor.IsValid() && CachedMountainActor->bEnableOnScreenMessages)
    {
        DrawDebugInfo();
    }
#endif
}

void ADownfallCharacter::ToggleFlyMode()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    UCapsuleComponent* Capsule = GetCapsuleComponent();

    if (!MoveComp || !Capsule)
    {
        return;
    }

    if (bIsClimbing)
    {
        if (LeftHand.State == EHandState::Gripping)
        {
            ReleaseGrip(true);
        }

        if (RightHand.State == EHandState::Gripping)
        {
            ReleaseGrip(false);
        }

        UpdateClimbingState();
    }

    Capsule->SetSimulatePhysics(false);
    Capsule->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Capsule->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    MoveComp->StopMovementImmediately();

    bDebugFlyMode = !bDebugFlyMode;

    if (bDebugFlyMode)
    {
        MoveComp->GravityScale = 0.0f;
        MoveComp->BrakingDecelerationFlying = DebugFlySpeed * 2.0f;
        MoveComp->MaxFlySpeed = DebugFlySpeed;
        MoveComp->SetMovementMode(MOVE_Flying);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("Fly Mode ON"));
        }

        UE_LOG(LogDownFall, Warning, TEXT("Fly Mode ON"));
    }
    else
    {
        MoveComp->GravityScale = 1.0f;
        MoveComp->SetMovementMode(MOVE_Walking);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("Fly Mode OFF"));
        }

        UE_LOG(LogDownFall, Warning, TEXT("Fly Mode OFF"));
    }
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

        if (IsValid(PauseAction))
        {
            EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &ADownfallCharacter::OnPauseTriggered);
        }
    }

    PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ADownfallCharacter::ToggleFlyMode);
    PlayerInputComponent->BindKey(EKeys::NumPadThree, IE_Pressed, this, &ADownfallCharacter::ToggleFlyMode);
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

bool ADownfallCharacter::TryAttachSafetyLineFromLookTarget(float TraceDistanceCm)
{
    FVector ViewLoc = GetActorLocation();
    FRotator ViewRot = GetActorRotation();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->GetPlayerViewPoint(ViewLoc, ViewRot);
    }
    else if (FirstPersonCamera)
    {
        ViewLoc = FirstPersonCamera->GetComponentLocation();
        ViewRot = FirstPersonCamera->GetComponentRotation();
    }

    const FVector Start = ViewLoc;
    const FVector End = Start + ViewRot.Vector() * TraceDistanceCm;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(SafetyLineTrace), false, this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    if (!bHit || !Hit.bBlockingHit)
    {
        return false;
    }

    AActor* AnchorActor = Hit.GetActor();
    if (!AnchorActor || !IsValid(AnchorActor))
    {
        return false;
    }

    if (!AnchorActor->ActorHasTag(TEXT("Anchor")))
    {
        return false;
    }

    return AttachSafetyLineToBolt(AnchorActor);
}

bool ADownfallCharacter::AttachSafetyLineToBolt(AActor* AnchorActor)
{
    if (!AnchorActor || !IsValid(AnchorActor))
    {
        return false;
    }

    // 앵커 블루프린트에는 반드시 Anchor 태그를 넣어둘 것
    if (!AnchorActor->ActorHasTag(TEXT("Anchor")))
    {
        return false;
    }

    if (bSafetyLineAttached)
    {
        DetachSafetyLine(false);
    }

    const FVector AnchorLoc = ResolveSafetyLineAnchorLocation(AnchorActor);
    if (AnchorLoc.IsNearlyZero())
    {
        return false;
    }

    ActiveSafetyBolt = AnchorActor;
    SafetyLineAnchorWorld = AnchorLoc;
    SafetyLineCurrentLengthCm = SafetyLineInitialLengthCm;
    bSafetyLineAttached = true;
    bSafetyLineConstraintEngaged = false;

    return true;
}

bool ADownfallCharacter::BeginUsingAnchorSlot(int32 SlotIndex)
{
    if (!Inventory || !Inventory->HasValidItemAt(SlotIndex))
    {
        return false;
    }

    if (!Inventory->EnsureAnchorDurabilityInitialized(SlotIndex, 5))
    {
        return false;
    }

    if (Inventory->GetAnchorDurabilityAt(SlotIndex) <= 0)
    {
        return false;
    }

    ActiveUsingAnchorSlotIndex = SlotIndex;
    RefreshInventoryUIState();
    return true;
}

bool ADownfallCharacter::IsInventorySlotUsing(int32 Index) const
{
    return Index != INDEX_NONE && Index == ActiveUsingAnchorSlotIndex && bSafetyLineAttached;
}

int32 ADownfallCharacter::GetDisplayedInventoryCountAt(int32 Index) const
{
    if (!Inventory || !Inventory->HasValidItemAt(Index))
    {
        return 0;
    }

    if (IsInventorySlotUsing(Index))
    {
        return Inventory->GetAnchorDurabilityAt(Index);
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();
    return Slots.IsValidIndex(Index) ? Slots[Index].Count : 0;
}

void ADownfallCharacter::DetachSafetyLine(bool bBreakBolt)
{
    if (SafetyLineConstraint)
    {
        SafetyLineConstraint->BreakConstraint();
    }

    bSafetyLineAttached = false;
    bSafetyLineConstraintEngaged = false;
    ActiveSafetyBolt = nullptr;
    SafetyLineAnchorWorld = FVector::ZeroVector;
    SafetyLineCurrentLengthCm = SafetyLineInitialLengthCm;
    ActiveUsingAnchorSlotIndex = INDEX_NONE;
    RefreshInventoryUIState();

    if (!bIsClimbing && AreBothHandsFree())
    {
        if (GetCharacterMovement()->MovementMode == MOVE_None)
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        }
    }
}

FVector ADownfallCharacter::GetSafetyLineAnchorLocation() const
{
    if (ActiveSafetyBolt && IsValid(ActiveSafetyBolt))
    {
        return ResolveSafetyLineAnchorLocation(ActiveSafetyBolt);
    }

    return SafetyLineAnchorWorld;
}

FVector ADownfallCharacter::ResolveSafetyLineAnchorLocation(const AActor* AnchorActor) const
{
    if (!AnchorActor || !IsValid(AnchorActor))
    {
        return FVector::ZeroVector;
    }

    // 1순위: RopeAttachPoint 태그가 붙은 SceneComponent 찾기
    TArray<USceneComponent*> Components;
    AnchorActor->GetComponents<USceneComponent>(Components);

    for (USceneComponent* SceneComp : Components)
    {
        if (!SceneComp)
        {
            continue;
        }

        if (SceneComp->ComponentHasTag(TEXT("RopeAttachPoint")))
        {
            return SceneComp->GetComponentLocation();
        }
    }

    // 2순위: 이름이 RopeAttachPoint인 컴포넌트 찾기
    for (USceneComponent* SceneComp : Components)
    {
        if (!SceneComp)
        {
            continue;
        }

        if (SceneComp->GetName().Contains(TEXT("RopeAttachPoint")))
        {
            return SceneComp->GetComponentLocation();
        }
    }

    // 3순위: 액터 위치 fallback
    return AnchorActor->GetActorLocation();
}

float ADownfallCharacter::GetSafetyLineAnchorDurability() const
{
    if (!Inventory || ActiveUsingAnchorSlotIndex == INDEX_NONE)
    {
        return 0.0f;
    }

    return (float)Inventory->GetAnchorDurabilityAt(ActiveUsingAnchorSlotIndex);
}

bool ADownfallCharacter::ConsumeSafetyLineAnchorDurability(float Amount)
{
    if (!Inventory || ActiveUsingAnchorSlotIndex == INDEX_NONE)
    {
        return false;
    }

    if (Amount <= 0.0f)
    {
        return true;
    }

    const int32 Remaining = Inventory->ConsumeAnchorUseAt(ActiveUsingAnchorSlotIndex, FMath::Max(1, FMath::RoundToInt(Amount)));
    return Remaining > 0;
}

void ADownfallCharacter::FinishUsingAnchor(bool bConsumeOneUse)
{
    const int32 UsedSlot = ActiveUsingAnchorSlotIndex;
    AActor* UsedAnchorActor = ActiveSafetyBolt.Get();

    if (bConsumeOneUse && Inventory && UsedSlot != INDEX_NONE)
    {
        Inventory->ConsumeAnchorUseAt(UsedSlot, 1);
    }

    if (UsedAnchorActor && IsValid(UsedAnchorActor))
    {
        UsedAnchorActor->Destroy();
    }

    DetachSafetyLine(false);

    if (UsedSlot != INDEX_NONE && HeldSlotIndex == UsedSlot && (!Inventory || !Inventory->HasValidItemAt(UsedSlot)))
    {
        HeldSlotIndex = INDEX_NONE;
        if (ItemUseState == EItemUseState::HoldingItem)
        {
            ItemUseState = EItemUseState::None;
        }
    }

    RefreshInventoryUIState();
}

bool ADownfallCharacter::IsSafetyLineTaut() const
{
    if (!bSafetyLineAttached)
    {
        return false;
    }

    const float CurrentDistance =
        FVector::Distance(GetCapsuleComponent()->GetComponentLocation(), GetSafetyLineAnchorLocation());

    return CurrentDistance >= (SafetyLineCurrentLengthCm - SafetyLineEngageToleranceCm);
}

void ADownfallCharacter::EngageSafetyLineConstraint()
{
    if (!bSafetyLineAttached || !SafetyLineConstraint)
    {
        return;
    }

    if (!GetCapsuleComponent()->IsSimulatingPhysics())
    {
        GetCapsuleComponent()->SetSimulatePhysics(true);
    }

    GetCharacterMovement()->SetMovementMode(MOVE_None);

    RefreshSafetyLineConstraint();
    bSafetyLineConstraintEngaged = true;
}

void ADownfallCharacter::DisengageSafetyLineConstraint()
{
    if (!bSafetyLineConstraintEngaged || !SafetyLineConstraint)
    {
        return;
    }

    SafetyLineConstraint->BreakConstraint();
    bSafetyLineConstraintEngaged = false;
}

void ADownfallCharacter::RefreshSafetyLineConstraint()
{
    if (!bSafetyLineAttached || !SafetyLineConstraint)
    {
        return;
    }

    const FVector Anchor = GetSafetyLineAnchorLocation();
    SafetyLineConstraint->SetWorldLocation(Anchor);

    SafetyLineConstraint->SetConstrainedComponents(
        GetCapsuleComponent(),
        NAME_None,
        nullptr,
        NAME_None
    );

    SafetyLineConstraint->SetLinearPositionDrive(false, false, false);
    SafetyLineConstraint->SetLinearVelocityDrive(false, false, false);

    SafetyLineConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, SafetyLineCurrentLengthCm);
    SafetyLineConstraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, SafetyLineCurrentLengthCm);
    SafetyLineConstraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, SafetyLineCurrentLengthCm);

    SafetyLineConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Free, 0.0f);
    SafetyLineConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Free, 0.0f);
    SafetyLineConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Free, 0.0f);
}

void ADownfallCharacter::UpdateSafetyLine(float DeltaTime)
{
    if (!bSafetyLineAttached)
    {
        return;
    }

    if (!ActiveSafetyBolt || !IsValid(ActiveSafetyBolt))
    {
        DetachSafetyLine(true);
        return;
    }

    // 앵커 액터가 더 이상 Anchor 태그가 아니면 무효 처리
    if (!ActiveSafetyBolt->ActorHasTag(TEXT("Anchor")))
    {
        DetachSafetyLine(true);
        return;
    }

    SafetyLineAnchorWorld = GetSafetyLineAnchorLocation();

    const FVector PlayerLoc = GetCapsuleComponent()->GetComponentLocation();
    const float CurrentDistance = FVector::Distance(PlayerLoc, SafetyLineAnchorWorld);

    if (bIsClimbing)
    {
        const float DesiredLength = FMath::Max(
            SafetyLineMinLengthCm,
            CurrentDistance + SafetyLineSlackCm
        );

        if (DesiredLength < SafetyLineCurrentLengthCm)
        {
            SafetyLineCurrentLengthCm = DesiredLength;

            if (bSafetyLineConstraintEngaged)
            {
                RefreshSafetyLineConstraint();
            }
        }

        const bool bReachedRetrievalPoint =
            (CurrentDistance <= (SafetyLineMinLengthCm + SafetyLineSlackCm + SafetyLineEngageToleranceCm));

        if (bReachedRetrievalPoint)
        {
            FinishUsingAnchor(true);
            return;
        }

        if (!IsSafetyLineTaut())
        {
            DisengageSafetyLineConstraint();
        }
    }

    const bool bIsFalling = (GetCharacterMovement()->MovementMode == MOVE_Falling);

    if (!bIsClimbing && bIsFalling)
    {
        if (CurrentDistance >= (SafetyLineCurrentLengthCm - SafetyLineEngageToleranceCm))
        {
            if (!bSafetyLineConstraintEngaged)
            {
                EngageSafetyLineConstraint();
            }
            else
            {
                RefreshSafetyLineConstraint();
            }
        }
    }

    if (bSafetyLineConstraintEngaged)
    {
        const FVector Dir = (PlayerLoc - SafetyLineAnchorWorld).GetSafeNormal();
        FVector Vel = GetCapsuleComponent()->GetPhysicsLinearVelocity();

        const float OutwardSpeed = FVector::DotProduct(Vel, Dir);
        if (OutwardSpeed > 0.0f && CurrentDistance >= (SafetyLineCurrentLengthCm - 5.0f))
        {
            Vel -= Dir * (OutwardSpeed * 0.85f);
            GetCapsuleComponent()->SetPhysicsLinearVelocity(Vel);
        }
    }

    if (!bIsClimbing && GetCharacterMovement()->MovementMode == MOVE_Walking)
    {
        DisengageSafetyLineConstraint();
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

    // 등반 중: Grip 조합에 따라 이동
    if (bIsClimbing)
    {
        UCapsuleComponent* Capsule = GetCapsuleComponent();
        if (!IsValid(Capsule)) return;

        const FHandData* PrimaryHand = GetPrimaryMovementHand();
        if (!PrimaryHand || !PrimaryHand->CurrentGrip.bIsValid)
        {
            return;
        }

        float ClimbSpeed = 500.0f;
        ClimbSpeed *= GetAnchorAssistMoveScale();

        // 이동 기준 Normal:
        // 1순위 Surface 손
        // 2순위 Anchor 손
        // 3순위 기타 grip
        FVector SurfaceNormal = PrimaryHand->CurrentGrip.SurfaceNormal.GetSafeNormal();
        if (SurfaceNormal.IsNearlyZero())
        {
            SurfaceNormal = FVector::ForwardVector;
        }

        // 벽면에 평행한 "위" 방향 계산
        FVector SurfaceUp = FVector::UpVector - SurfaceNormal * FVector::DotProduct(FVector::UpVector, SurfaceNormal);
        if (!SurfaceUp.Normalize())
        {
            SurfaceUp = FVector::CrossProduct(SurfaceNormal, FVector::RightVector).GetSafeNormal();
        }

        // 벽면에 평행한 "오른쪽" 방향 계산
        FVector SurfaceRight = FVector::CrossProduct(SurfaceNormal, SurfaceUp).GetSafeNormal();

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

        // 앵커가 포함되면 더 안정적으로 감쇠
        const float InterpSpeed = GetAnchorAssistInterpSpeed();

        FVector NewVelocity = FMath::VInterpTo(
            CurrentVelocity,
            TargetVelocity,
            GetWorld()->GetDeltaSeconds(),
            InterpSpeed
        );

        // 입력이 없고 앵커가 걸려 있으면 더 빨리 안정화
        if (HasAnchorGrip() && MovementVector.IsNearlyZero())
        {
            NewVelocity *= 0.85f;
        }

        Capsule->SetPhysicsLinearVelocity(NewVelocity);
    }
    // 지상: 일반 이동
    else
    {
        if (MovementVector.Y != 0.0f)
        {
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            AddMovementInput(Direction, MovementVector.Y);
        }

        if (MovementVector.X != 0.0f)
        {
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
            AddMovementInput(Direction, MovementVector.X);
        }
    }
}

void ADownfallCharacter::OnJumpStarted(const FInputActionValue& Value)
{
    if (bDebugFlyMode)
    {
        return;
    }

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
    if (bDebugFlyMode)
    {
        return;
    }

    // 일반 점프 종료
    StopJumping();
}

// Debug: Insanity Test
void ADownfallCharacter::OnDebugInsanity(const FInputActionValue& Value)
{
    AddInsanity(10.0f);
    UE_LOG(LogDownFall, Warning, TEXT("Debug: Added 10 Insanity (Test key pressed)"));
}

// Anchor Grip Logic
void ADownfallCharacter::EnterAnchorGrip(bool bIsLeftHand, const FGripPointInfo& GripInfo)
{
    CurrentAnchorActor = GripInfo.SourceActor;

    if (bIsLeftHand)
    {
        bAnchorGripLeftHand = true;
    }
    else
    {
        bAnchorGripRightHand = true;
    }

    // 현재 대표 앵커 정보 캐시
    CurrentAnchorPointWorld = GripInfo.WorldLocation;
    CurrentAnchorNormal = GripInfo.SurfaceNormal.GetSafeNormal();

    UE_LOG(LogDownFall, Log, TEXT("EnterAnchorGrip: %s"), *GetNameSafe(CurrentAnchorActor));
}

void ADownfallCharacter::UpdateAnchorGrip(float DeltaTime)
{
    if (!IsAnchorGripActive())
    {
        CurrentAnchorActor = nullptr;
        CurrentAnchorPointWorld = FVector::ZeroVector;
        CurrentAnchorNormal = FVector::UpVector;
        return;
    }

    FVector SumPoint = FVector::ZeroVector;
    FVector SumNormal = FVector::ZeroVector;
    int32 Count = 0;

    if (LeftHand.State == EHandState::Gripping &&
        LeftHand.CurrentGrip.GripKind == EGripKind::Anchor &&
        LeftHand.CurrentGrip.bIsValid)
    {
        SumPoint += LeftHand.CurrentGrip.WorldLocation;
        SumNormal += LeftHand.CurrentGrip.SurfaceNormal.GetSafeNormal();
        ++Count;
    }

    if (RightHand.State == EHandState::Gripping &&
        RightHand.CurrentGrip.GripKind == EGripKind::Anchor &&
        RightHand.CurrentGrip.bIsValid)
    {
        SumPoint += RightHand.CurrentGrip.WorldLocation;
        SumNormal += RightHand.CurrentGrip.SurfaceNormal.GetSafeNormal();
        ++Count;
    }

    if (Count > 0)
    {
        CurrentAnchorPointWorld = SumPoint / (float)Count;
        CurrentAnchorNormal = (SumNormal / (float)Count).GetSafeNormal();
    }
}

void ADownfallCharacter::ExitAnchorGrip(bool bIsLeftHand)
{
    if (bIsLeftHand)
    {
        bAnchorGripLeftHand = false;
    }
    else
    {
        bAnchorGripRightHand = false;
    }

    if (!bAnchorGripLeftHand && !bAnchorGripRightHand)
    {
        CurrentAnchorActor = nullptr;
        CurrentAnchorPointWorld = FVector::ZeroVector;
        CurrentAnchorNormal = FVector::UpVector;

        UE_LOG(LogDownFall, Log, TEXT("ExitAnchorGrip"));
    }
}

bool ADownfallCharacter::IsAnchorGripActive() const
{
    return bAnchorGripLeftHand || bAnchorGripRightHand;
}

bool ADownfallCharacter::HasAnchorGrip() const
{
    const bool bLeftAnchor =
        (LeftHand.State == EHandState::Gripping &&
            LeftHand.CurrentGrip.GripKind == EGripKind::Anchor);

    const bool bRightAnchor =
        (RightHand.State == EHandState::Gripping &&
            RightHand.CurrentGrip.GripKind == EGripKind::Anchor);

    return bLeftAnchor || bRightAnchor;
}

bool ADownfallCharacter::HasSurfaceGrip() const
{
    const bool bLeftSurface =
        (LeftHand.State == EHandState::Gripping &&
            LeftHand.CurrentGrip.GripKind == EGripKind::Surface);

    const bool bRightSurface =
        (RightHand.State == EHandState::Gripping &&
            RightHand.CurrentGrip.GripKind == EGripKind::Surface);

    return bLeftSurface || bRightSurface;
}

bool ADownfallCharacter::HasDynamicGrip() const
{
    const bool bLeftDynamic =
        (LeftHand.State == EHandState::Gripping &&
            LeftHand.CurrentGrip.GripKind == EGripKind::DynamicActor);

    const bool bRightDynamic =
        (RightHand.State == EHandState::Gripping &&
            RightHand.CurrentGrip.GripKind == EGripKind::DynamicActor);

    return bLeftDynamic || bRightDynamic;
}

const FHandData* ADownfallCharacter::GetPrimaryMovementHand() const
{
    // 1순위: Surface 손
    if (LeftHand.State == EHandState::Gripping &&
        LeftHand.CurrentGrip.GripKind == EGripKind::Surface &&
        LeftHand.CurrentGrip.bIsValid)
    {
        return &LeftHand;
    }

    if (RightHand.State == EHandState::Gripping &&
        RightHand.CurrentGrip.GripKind == EGripKind::Surface &&
        RightHand.CurrentGrip.bIsValid)
    {
        return &RightHand;
    }

    // 2순위: Anchor 손
    if (LeftHand.State == EHandState::Gripping &&
        LeftHand.CurrentGrip.GripKind == EGripKind::Anchor &&
        LeftHand.CurrentGrip.bIsValid)
    {
        return &LeftHand;
    }

    if (RightHand.State == EHandState::Gripping &&
        RightHand.CurrentGrip.GripKind == EGripKind::Anchor &&
        RightHand.CurrentGrip.bIsValid)
    {
        return &RightHand;
    }

    // 3순위: 기타 grip
    if (LeftHand.State == EHandState::Gripping && LeftHand.CurrentGrip.bIsValid)
    {
        return &LeftHand;
    }

    if (RightHand.State == EHandState::Gripping && RightHand.CurrentGrip.bIsValid)
    {
        return &RightHand;
    }

    return nullptr;
}

float ADownfallCharacter::GetAnchorAssistMoveScale() const
{
    // 절벽만 잡고 있으면 기존 속도 유지
    if (!HasAnchorGrip())
    {
        return 1.0f;
    }

    // 절벽 + 앵커 혼합
    if (HasAnchorGrip() && HasSurfaceGrip())
    {
        return 0.80f;
    }

    // 양손 앵커 또는 앵커만 잡고 있는 상태
    return 0.65f;
}

float ADownfallCharacter::GetAnchorAssistInterpSpeed() const
{
    // 기본 surface 이동보다 anchor가 있으면 좀 더 빠르게 안정화
    if (!HasAnchorGrip())
    {
        return 5.0f;
    }

    if (HasAnchorGrip() && HasSurfaceGrip())
    {
        return 8.0f;
    }

    return 10.0f;
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
    Hand.GrippedActor = nullptr;
    Hand.GripStartTime = GetWorld()->GetTimeSeconds();

    // 6. 그립 종류별 처리
    switch (GripInfo.GripKind)
    {
    case EGripKind::Anchor:
        // 앵커도 월드 고정점처럼 Constraint를 건다.
        // 다만 locomotion은 잠그지 않고, Anchor 보조점으로 등록한다.
        SetupConstraint(Constraint, GripInfo.WorldLocation);
        EnterAnchorGrip(bIsLeftHand, GripInfo);
        break;

    case EGripKind::DynamicActor:
    {
        if (AFlyingPlatform* Platform = Cast<AFlyingPlatform>(GripInfo.SourceActor))
        {
            Hand.GrippedActor = Platform;

            Platform->OnPlayerGrab(this);
            UE_LOG(LogDownFall, Log, TEXT("Grabbed Flying Platform: %s"), *Platform->GetName());

            SetupConstraintToActor(Constraint, Platform, GripInfo.WorldLocation);
        }
        else
        {
            // 안전 fallback
            SetupConstraint(Constraint, GripInfo.WorldLocation);
        }
        break;
    }

    case EGripKind::Surface:
    default:
        SetupConstraint(Constraint, GripInfo.WorldLocation);
        break;
    }

    // 7. 물리 모드 전환 (첫 Grip인 경우)
    if (bWasBothHandsFree)
    {
        GetCapsuleComponent()->SetSimulatePhysics(true);
        GetCharacterMovement()->SetMovementMode(MOVE_None);

        UE_LOG(LogDownFall, Log, TEXT("Physics mode activated"));
    }

    // 8. 등반 상태 업데이트
    UpdateClimbingState();

    // 9. 이벤트 발생
    OnHandGripped(bIsLeftHand, GripInfo);

    UE_LOG(LogDownFall, Log, TEXT("%s hand gripped: Kind=%d Angle=%.1f°, Quality=%.2f"),
        bIsLeftHand ? TEXT("Left") : TEXT("Right"),
        static_cast<int32>(GripInfo.GripKind),
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

    // 1. 앵커 상태 해제
    if (Hand.CurrentGrip.GripKind == EGripKind::Anchor)
    {
        ExitAnchorGrip(bIsLeftHand);
    }

    // 2. 동적 액터 해제
    if (Hand.GrippedActor)
    {
        if (AFlyingPlatform* Platform = Cast<AFlyingPlatform>(Hand.GrippedActor))
        {
            Platform->OnPlayerRelease();
            UE_LOG(LogDownFall, Log, TEXT("Released Flying Platform: %s"), *Platform->GetName());
        }

        Hand.GrippedActor = nullptr;
    }

    // 3. Constraint 해제
    BreakConstraint(Constraint);

    // 4. 상태 업데이트
    Hand.State = EHandState::Free;
    Hand.CurrentGrip = FGripPointInfo();

    // 5. 등반 상태 업데이트
    UpdateClimbingState();

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

    // 경사각 기반 배율 계산
    float Angle = Hand.CurrentGrip.SurfaceAngleDegrees;
    float AngleMultiplier = 1.0f;

    if (Angle < CeilingAngleThreshold)
    {
        AngleMultiplier = CeilingAngleMultiplier;
    }
    else if (Angle < SteepAngleThreshold)
    {
        AngleMultiplier = SteepAngleMultiplier;
    }
    else if (Angle < VerticalAngleThreshold)
    {
        AngleMultiplier = VerticalAngleMultiplier;
    }
    else if (Angle < OverhangAngleThreshold)
    {
        AngleMultiplier = OverhangAngleMultiplier;
    }
    else if (Angle < SteepOverhangAngleThreshold)
    {
        AngleMultiplier = SteepOverhangAngleMultiplier;
    }
    else
    {
        AngleMultiplier = FloorAngleMultiplier;
    }

    BaseDrain *= AngleMultiplier;

    // 품질 기반 배율 계산
    float Quality = Hand.CurrentGrip.GripQuality;
    float QualityMultiplier = 1.0f;

    if (Quality >= AnchorQualityThreshold)
    {
        QualityMultiplier = AnchorQualityMultiplier;
    }
    else if (Quality >= GoodQualityThreshold)
    {
        QualityMultiplier = GoodQualityMultiplier;
    }
    else if (Quality >= PoorQualityThreshold)
    {
        QualityMultiplier = NormalQualityMultiplier;
    }
    else if (Quality >= VeryPoorQualityThreshold)
    {
        QualityMultiplier = PoorQualityMultiplier;
    }
    else
    {
        QualityMultiplier = VeryPoorQualityMultiplier;
    }

    BaseDrain *= QualityMultiplier;

    return BaseDrain;
}

bool ADownfallCharacter::CanRestoreStamina() const
{
    return (LeftHand.Stamina < MaxStamina) || (RightHand.Stamina < MaxStamina);
}

bool ADownfallCharacter::RestoreStamina(float Amount)
{
    if (Amount <= 0.f)
    {
        return false;
    }

    if (!CanRestoreStamina())
    {
        return false;
    }

    LeftHand.Stamina = FMath::Clamp(LeftHand.Stamina + Amount, 0.f, MaxStamina);
    RightHand.Stamina = FMath::Clamp(RightHand.Stamina + Amount, 0.f, MaxStamina);

    return true;
}

void ADownfallCharacter::AddInsanity(float Amount)
{
    Insanity = FMath::Clamp(Insanity + Amount, 0.0f, MaxInsanity);
    UpdateInsanityEffects();

    // Altitude Widget Glitch 모드 제어 (상태 변화 감지)
    if (AltitudeWidget)
    {
        bool bIsNowAboveThreshold = (Insanity >= AltitudeGlitchThreshold);

        // 상태가 변했을 때만 호출
        if (bIsNowAboveThreshold != bWasAboveGlitchThreshold)
        {
            if (bIsNowAboveThreshold)
            {
                // 80 이상으로 변경됨
                AltitudeWidget->EnableGlitchMode();
                UE_LOG(LogDownFall, Warning, TEXT("Insanity %.1f (+%.1f): Altitude Glitch ENABLED"), Insanity, Amount);
            }
            else
            {
                // 80 미만으로 변경됨 (음수로 감소 시)
                AltitudeWidget->DisableGlitchMode();
                UE_LOG(LogDownFall, Warning, TEXT("Insanity %.1f (+%.1f): Altitude Glitch DISABLED"), Insanity, Amount);
            }

            // 플래그 업데이트
            bWasAboveGlitchThreshold = bIsNowAboveThreshold;
        }
    }

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

    // Altitude Widget Glitch 모드 제어 (상태 변화 감지)
    if (AltitudeWidget)
    {
        bool bIsNowAboveThreshold = (Insanity >= AltitudeGlitchThreshold);

        // 상태가 변했을 때만 호출
        if (bIsNowAboveThreshold != bWasAboveGlitchThreshold)
        {
            if (bIsNowAboveThreshold)
            {
                // 80 이상으로 변경됨
                AltitudeWidget->EnableGlitchMode();
                UE_LOG(LogDownFall, Warning, TEXT("Insanity %.1f: Altitude Glitch ENABLED"), Insanity);
            }
            else
            {
                // 80 미만으로 변경됨
                AltitudeWidget->DisableGlitchMode();
                UE_LOG(LogDownFall, Warning, TEXT("Insanity %.1f: Altitude Glitch DISABLED"), Insanity);
            }

            // 플래그 업데이트
            bWasAboveGlitchThreshold = bIsNowAboveThreshold;
        }
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

    // 현재 grip 조합 요약
    if (bIsClimbing)
    {
        if (HasDynamicGrip())
        {
            GripMode = EGripMode::DynamicGrip;
        }
        else if (HasAnchorGrip() && !HasSurfaceGrip())
        {
            GripMode = EGripMode::AnchorGrip;
        }
        else
        {
            // Surface만 있거나, Surface + Anchor 혼합이면
            // locomotion은 surface 계열로 본다
            GripMode = EGripMode::SurfaceGrip;
        }
    }

    // 양손 모두 놓았을 때
    if (bWasClimbing && !bIsClimbing)
    {
        GripMode = EGripMode::None;

        bAnchorGripLeftHand = false;
        bAnchorGripRightHand = false;
        CurrentAnchorActor = nullptr;
        CurrentAnchorPointWorld = FVector::ZeroVector;
        CurrentAnchorNormal = FVector::UpVector;

        GetCapsuleComponent()->SetSimulatePhysics(false);

        // Rotation 초기화
        FRotator CurrentRotation = GetActorRotation();
        FRotator UpRightRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f);
        SetActorRotation(UpRightRotation);

        // 바닥에 서있는지 체크
        FHitResult Hit;
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 100.0f);

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
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(Hit.ImpactNormal.Z));
            float WalkableSlope = GetCharacterMovement()->GetWalkableFloorAngle();

            if (SlopeAngle <= WalkableSlope)
            {
                GetCapsuleComponent()->SetPhysicsLinearVelocity(FVector::ZeroVector);
                GetCapsuleComponent()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
                GetCharacterMovement()->Velocity = FVector::ZeroVector;

                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                UE_LOG(LogDownFall, Log, TEXT("Released on ground - Walking mode"));
            }
            else
            {
                GetCharacterMovement()->SetMovementMode(MOVE_Falling);
                UE_LOG(LogDownFall, Log, TEXT("Released on steep slope - Falling"));
            }
        }
        else
        {
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

    int32 LineIndex = 100;

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

    GEngine->AddOnScreenDebugMessage(
        LineIndex++, 0.0f, bDebugFlyMode ? FColor::Yellow : FColor::Silver,
        FString::Printf(TEXT("Fly Mode: %s | Space Up / Ctrl Down"), bDebugFlyMode ? TEXT("ON") : TEXT("OFF"))
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

#if 0
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

// Chromatic Aberration 포함
void ADownfallCharacter::UpdateLensDistortionEffect()
{
    if (!LensDistortionMaterialInstance)
        return;

    // Insanity 0~100 범위를 0~1로 정규화
    float InsanityNormalized = FMath::Clamp(Insanity / MaxInsanity, 0.0f, 1.0f);

    // K1 값 계산: Insanity 증가 시 BaseK1에서 MaxK1로 변화
    CurrentK1 = FMath::Lerp(BaseK1, MaxK1, InsanityNormalized);

    // Material Parameter 업데이트
    LensDistortionMaterialInstance->SetScalarParameterValue(FName("K1"), CurrentK1);
    LensDistortionMaterialInstance->SetScalarParameterValue(FName("K2"), K2);

    // Chromatic Aberration 업데이트
    CurrentChromaticAberration = FMath::Lerp(BaseChromaticAberration, MaxChromaticAberration, InsanityNormalized);

    if (PostProcessComp)
    {
        PostProcessComp->Settings.SceneFringeIntensity = CurrentChromaticAberration;
    }

    // FilmGrain 업데이트
    CurrentFilmGrainIntensity = FMath::Lerp(BaseFilmGrainIntensity, MaxFilmGrainIntensity, InsanityNormalized);

    if (PostProcessComp)
    {
        PostProcessComp->Settings.FilmGrainIntensity = CurrentFilmGrainIntensity;
    }
}

void ADownfallCharacter::UpdateVignetteEffect(float DeltaTime)
{
    if (!VignetteMaterialInstance || !FirstPersonCamera)
        return;

    // 카메라의 회전 속도 계산 (Pitch, Yaw)
    FRotator CameraRotation = FirstPersonCamera->GetComponentRotation();

    // 이전 프레임과의 회전 차이 계산
    static FRotator LastRotation = CameraRotation;
    FRotator RotationDelta = CameraRotation - LastRotation;
    LastRotation = CameraRotation;

    // 회전 속도에 비례한 오프셋 계산
    float PitchOffset = RotationDelta.Pitch * VignetteShiftAmount * DeltaTime;
    float YawOffset = RotationDelta.Yaw * VignetteShiftAmount * DeltaTime;

    // 오프셋 누적 (부드럽게 감쇠)
    CurrentVignetteOffset.X = FMath::FInterpTo(CurrentVignetteOffset.X, YawOffset, DeltaTime, 5.0f);
    CurrentVignetteOffset.Y = FMath::FInterpTo(CurrentVignetteOffset.Y, -PitchOffset, DeltaTime, 5.0f);

    // 오프셋 제한 (-0.1 ~ 0.1)
    CurrentVignetteOffset.X = FMath::Clamp(CurrentVignetteOffset.X, -0.1f, 0.1f);
    CurrentVignetteOffset.Y = FMath::Clamp(CurrentVignetteOffset.Y, -0.1f, 0.1f);

    // Material Parameter 업데이트
    VignetteMaterialInstance->SetVectorParameterValue(
        FName("VignetteOffset"),
        FLinearColor(CurrentVignetteOffset.X, CurrentVignetteOffset.Y, 0.0f)
    );
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

    RefreshInventoryUIState();
}

void ADownfallCharacter::CloseInventoryToEmptyHand()
{
    InventoryCursorIndex = 12;

    if (Inventory)
    {
        Inventory->SetPreviewEnabled(false);
    }

    if (IsValidInventorySlotIndex(HeldSlotIndex))
    {
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
    }
    else
    {
        HeldSlotIndex = INDEX_NONE;
        ItemUseState = EItemUseState::None;
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
        UWorld* W = GetWorld();
        UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
        UItemSubsystem* IS = GI ? GI->GetSubsystem<UItemSubsystem>() : nullptr;

        const UItemDefinition* Def = nullptr;
        if (IS && Slots.IsValidIndex(HeldSlotIndex))
        {
            Def = IS->GetItemDefinitionById(Slots[HeldSlotIndex].ItemId);
        }

        if (!Def || !Def->PlaceActorClass)
        {
            return false;
        }

        Inventory->SetPreviewActorClass(Def->PlaceActorClass);
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

    const bool bHasPreviewActorClass = (Def->PlaceActorClass != nullptr);

    return bHasPreviewActorClass &&
        (Def->UseType == EItemUseType::PlaceActor ||
            Def->UseType == EItemUseType::AttachAnchorToBolt);
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

void ADownfallCharacter::RefreshLowFrequencyUpdates()
{
    CheckForPlatformAbduction();
    UpdateAltitudeUI();
}

void ADownfallCharacter::StartLowFrequencyUpdatesIfNeeded()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const bool bNeedsPlatformCheck =
        (LeftHand.State == EHandState::Gripping || RightHand.State == EHandState::Gripping);

    const bool bNeedsAltitudeUI = (AltitudeWidget != nullptr);

    if (!bNeedsPlatformCheck && !bNeedsAltitudeUI)
    {
        return;
    }

    const float SafePlatformInterval = FMath::Max(0.02f, PlatformAbductionCheckInterval);
    const float SafeAltitudeInterval = FMath::Max(0.02f, AltitudeUpdateInterval);
    const float SafeInterval = FMath::Min(SafePlatformInterval, SafeAltitudeInterval);

    if (!World->GetTimerManager().IsTimerActive(LowFrequencyUpdateTimerHandle))
    {
        World->GetTimerManager().SetTimer(
            LowFrequencyUpdateTimerHandle,
            this,
            &ADownfallCharacter::RefreshLowFrequencyUpdates,
            SafeInterval,
            true
        );
    }
}

void ADownfallCharacter::StopLowFrequencyUpdatesIfPossible()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const bool bNeedsPlatformCheck =
        (LeftHand.State == EHandState::Gripping || RightHand.State == EHandState::Gripping);

    const bool bNeedsAltitudeUI = (AltitudeWidget != nullptr);

    if (!bNeedsPlatformCheck && !bNeedsAltitudeUI)
    {
        World->GetTimerManager().ClearTimer(LowFrequencyUpdateTimerHandle);
    }
}

void ADownfallCharacter::ApplyDirtMaskParameters(bool bForce)
{
    if (!DirtMaskMaterialInstance)
    {
        return;
    }

    auto ApplyIfChanged = [bForce](UMaterialInstanceDynamic* MID, const TCHAR* ParamName, float NewValue, float& CachedValue)
        {
            if (bForce || !FMath::IsNearlyEqual(CachedValue, NewValue))
            {
                MID->SetScalarParameterValue(FName(ParamName), NewValue);
                CachedValue = NewValue;
            }
        };

    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("DirtIntensity"), DirtIntensity, CachedDirtIntensity);
    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("BlurOffset"), DirtBlurOffset, CachedDirtBlurOffset);
    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("TintStrength"), DirtTintStrength, CachedDirtTintStrength);
    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("LightResponse"), DirtLightResponse, CachedDirtLightResponse);
    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("LightThreshold"), DirtLightThreshold, CachedDirtLightThreshold);
    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("LightSoftness"), DirtLightSoftness, CachedDirtLightSoftness);
    ApplyIfChanged(DirtMaskMaterialInstance, TEXT("DirtExposure"), DirtExposure, CachedDirtExposure);
}

void ADownfallCharacter::ApplyEdgeBlurParameters(bool bForce)
{
    if (!EdgeBlurMaterialInstance)
    {
        return;
    }

    auto ApplyIfChanged = [bForce](UMaterialInstanceDynamic* MID, const TCHAR* ParamName, float NewValue, float& CachedValue)
        {
            if (bForce || !FMath::IsNearlyEqual(CachedValue, NewValue))
            {
                MID->SetScalarParameterValue(FName(ParamName), NewValue);
                CachedValue = NewValue;
            }
        };

    ApplyIfChanged(EdgeBlurMaterialInstance, TEXT("BlurStart"), BlurStart, CachedBlurStart);
    ApplyIfChanged(EdgeBlurMaterialInstance, TEXT("BlurEnd"), BlurEnd, CachedBlurEnd);
    ApplyIfChanged(EdgeBlurMaterialInstance, TEXT("BlurStrength"), BlurStrength, CachedBlurStrength);
    ApplyIfChanged(EdgeBlurMaterialInstance, TEXT("BlurOffset"), BlurOffset, CachedBlurOffset);
}

void ADownfallCharacter::UpdateAltitudeUI()
{
    if (!AltitudeWidget)
    {
        return;
    }

    float CurrentZ = GetActorLocation().Z;
    float MaxHeight = 5000.0f;

    AMountainGenWorldActor* MountainActor = CachedMountainActor.Get();
    if (!MountainActor)
    {
        MountainActor = FindMountainGenActor();
        CachedMountainActor = MountainActor;
    }

    if (MountainActor)
    {
        MaxHeight = MountainActor->Settings.CliffHeightCm;
    }

    AltitudeWidget->UpdateAltitude(CurrentZ, InitialGroundHeight, MaxHeight);
}

AMountainGenWorldActor* ADownfallCharacter::FindMountainGenActor()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMountainGenWorldActor::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        AMountainGenWorldActor* Found = Cast<AMountainGenWorldActor>(FoundActors[0]);
        if (Found)
        {
            UE_LOG(LogDownFall, Log, TEXT("MountainGenWorldActor found: %s"), *Found->GetName());
        }
        return Found;
    }

    UE_LOG(LogDownFall, Warning, TEXT("MountainGenWorldActor not found in level"));
    return nullptr;
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

void ADownfallCharacter::UpdateHandStaminaVisuals(float DeltaTime)
{
    // 왼손 업데이트
    if (LeftHandMaterialInstance && LeftHandMesh)
    {
        UpdateHandMaterial(LeftHandMaterialInstance, LeftHand.Stamina);
        UpdateHandShake(LeftHandMesh, LeftHand.Stamina, DeltaTime, LeftHandRestPosition);
    }

    // 오른손 업데이트
    if (RightHandMaterialInstance && RightHandMesh)
    {
        UpdateHandMaterial(RightHandMaterialInstance, RightHand.Stamina);
        UpdateHandShake(RightHandMesh, RightHand.Stamina, DeltaTime, RightHandRestPosition);
    }
}

void ADownfallCharacter::UpdateHandMaterial(UMaterialInstanceDynamic* MaterialInstance, float Stamina)
{
    if (!MaterialInstance)
        return;

    // 스태미나 값을 0~1 범위로 정규화
    float StaminaPercent = FMath::Clamp(Stamina / 100.0f, 0.0f, 1.0f);

    float ColorBlendStrength = 0.0f;

    if (StaminaPercent >= 0.6f)
    {
        ColorBlendStrength = 0.0f;
    }
    else if (StaminaPercent >= 0.3f)
    {
        ColorBlendStrength = 0.65f;
    }
    else
    {
        ColorBlendStrength = 0.95f;
    }

    // Material Parameter 설정
    MaterialInstance->SetScalarParameterValue(FName("StaminaBlend"), ColorBlendStrength);
}

void ADownfallCharacter::UpdateHandShake(USkeletalMeshComponent* HandMesh, float Stamina, float DeltaTime, const FVector& RestPosition)
{
    if (!HandMesh)
        return;

    float StaminaPercent = FMath::Clamp(Stamina / 100.0f, 0.0f, 1.0f);

    // 25% 이하일 때만 떨림 추가
    if (StaminaPercent <= 0.25f)
    {
        HandShakeTimer += DeltaTime * HandShakeSpeed;

        // 떨림 강도 (스태미나가 낮을수록 강하게)
        float ShakeAmount = (1.0f - (StaminaPercent / 0.25f)) * HandShakeIntensity;

        // Sin/Cos으로 자연스러운 떨림 생성
        FVector ShakeOffset;
        ShakeOffset.X = FMath::Sin(HandShakeTimer * 1.3f) * ShakeAmount;
        ShakeOffset.Y = FMath::Cos(HandShakeTimer * 1.7f) * ShakeAmount;
        ShakeOffset.Z = FMath::Sin(HandShakeTimer * 2.1f) * ShakeAmount * 0.5f;

        // 현재 위치 가져오기
        FVector CurrentLocation = HandMesh->GetRelativeLocation();

        // 떨림만 추가 (작은 값으로)
        HandMesh->SetRelativeLocation(CurrentLocation + ShakeOffset * 0.1f);
    }
    // 스태미나 25% 이상일 때는 아무것도 안 함 (UpdateHandPositions가 정상 작동)
}

void ADownfallCharacter::OnPauseTriggered(const FInputActionValue& Value)
{
    // 이미 일시정지 중이면 무시
    if (UGameplayStatics::IsGamePaused(GetWorld()))
    {
        return;
    }

    if (!PauseMenuWidgetClass)
    {
        UE_LOG(LogDownFall, Warning, TEXT("PauseMenuWidgetClass not set!"));
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
        return;

    // Pause Widget 생성
    PauseMenuWidget = CreateWidget<UPauseMenuWidget>(PC, PauseMenuWidgetClass);
    if (PauseMenuWidget)
    {
        PauseMenuWidget->AddToViewport(100);  // 최상위

        // 게임 일시정지
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        UE_LOG(LogDownFall, Warning, TEXT("Game Paused"));
    }
}

void ADownfallCharacter::UpdateDirtMaskEffect()
{
    if (!DirtMaskMaterialInstance)
        return;

    // Material Parameter 업데이트 (Details에서 설정한 값 그대로 적용)
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("DirtIntensity"), DirtIntensity);
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("BlurOffset"), DirtBlurOffset);
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("TintStrength"), DirtTintStrength);
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("LightResponse"), DirtLightResponse);
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("LightThreshold"), DirtLightThreshold);
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("LightSoftness"), DirtLightSoftness);
    DirtMaskMaterialInstance->SetScalarParameterValue(FName("DirtExposure"), DirtExposure);
}

void ADownfallCharacter::UpdateEdgeBlurEffect()
{
    if (!EdgeBlurMaterialInstance)
        return;

    // Material Parameter 업데이트 (Details에서 설정한 값 그대로 적용)
    EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurStart"), BlurStart);
    EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurEnd"), BlurEnd);
    EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurStrength"), BlurStrength);
    EdgeBlurMaterialInstance->SetScalarParameterValue(FName("BlurOffset"), BlurOffset);
}