// File: Source/Prototype/Private/Core/CliffSelectionPawn.cpp
#include "Core/CliffSelectionPawn.h"
#include "Core/CliffSelectionGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "UI/CliffSelectionHUDWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "MountainGenWorldActor.h"
#include "MountainGenSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ACliffSelectionPawn::ACliffSelectionPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);

    bUseControllerRotationYaw = false;
}

void ACliffSelectionPawn::BeginPlay()
{
    Super::BeginPlay();

    // 초기 Yaw / 패닝 상태 초기화
    TargetYaw = GetActorRotation().Yaw;
    PanStartZ = GetActorLocation().Z;
    PanCurrentDistance = 0.f;

    // HUD 생성
    if (HUDWidgetClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            HUDWidget = CreateWidget<UCliffSelectionHUDWidget>(PC, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport();
                HUDWidget->SetInputHintVisible(false);
            }
        }
    }

    // GameMode의 OnAllCliffsGenerated 구독 -> 생성 완료 시 초기 록온/HUD 갱신
    if (ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        GM->OnAllCliffsGenerated.AddDynamic(this, &ACliffSelectionPawn::HandleAllCliffsGenerated);

        // GameMode BeginPlay가 먼저 실행되어 브로드캐스트를 이미 놓친 경우 즉시 처리
        if (GM->IsAllCliffsGenerated())
        {
            HandleAllCliffsGenerated();
        }
    }
}

void ACliffSelectionPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (CliffSelectionMappingContext)
            {
                Subsystem->AddMappingContext(CliffSelectionMappingContext, 0);
            }
        }
    }

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveSelectionLeftAction)
        {
            EIC->BindAction(MoveSelectionLeftAction, ETriggerEvent::Started, this, &ACliffSelectionPawn::OnMoveSelectionLeft);
        }
        if (MoveSelectionRightAction)
        {
            EIC->BindAction(MoveSelectionRightAction, ETriggerEvent::Started, this, &ACliffSelectionPawn::OnMoveSelectionRight);
        }
        if (ConfirmSelectionAction)
        {
            EIC->BindAction(ConfirmSelectionAction, ETriggerEvent::Started, this, &ACliffSelectionPawn::OnConfirmSelection);
        }
    }
}

void ACliffSelectionPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickCameraRotate(DeltaTime);

    if (bHasGeneratedComplete)
    {
        TickCameraPan(DeltaTime);
    }
}

void ACliffSelectionPawn::OnMoveSelectionLeft(const FInputActionValue& Value)
{
    if (!bHasGeneratedComplete) return;
    if (CurrentIndex <= 0) return;

    StartCameraRotateTo(CurrentIndex - 1);
}

void ACliffSelectionPawn::OnMoveSelectionRight(const FInputActionValue& Value)
{
    if (!bHasGeneratedComplete) return;

    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    const int32 MaxIndex = GM ? (GM->GetSpawnedCliffs().Num() - 1) : 2;

    if (CurrentIndex >= MaxIndex) return;

    StartCameraRotateTo(CurrentIndex + 1);
}

void ACliffSelectionPawn::OnConfirmSelection(const FInputActionValue& Value)
{
    if (!bHasGeneratedComplete) return;

    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (!GI) return;

    const TArray<int32>& Seeds = GI->GetGeneratedSeeds();
    if (!Seeds.IsValidIndex(CurrentIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionPawn: Invalid CurrentIndex for seed selection (%d)"), CurrentIndex);
        return;
    }

    GI->SetSelectedSeed(Seeds[CurrentIndex]);
    GI->SetCurrentStageIndex(GI->GetCurrentStageIndex() + 1);

    if (NextStageLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionPawn: NextStageLevelName not set"));
        return;
    }

    UGameplayStatics::OpenLevel(this, NextStageLevelName);
}

void ACliffSelectionPawn::StartCameraRotateTo(int32 NewIndex)
{
    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    const TArray<float>& Angles = GM->GetCliffAnglesDeg();
    if (!Angles.IsValidIndex(NewIndex))
    {
        return;
    }

    CurrentIndex = NewIndex;
    TargetYaw = Angles[NewIndex];
    bIsRotating = true;

    // 패닝 타겟 거리 갱신 (새로 록온된 암벽의 CliffHeightCm), 패닝 위치 리셋
    if (AMountainGenWorldActor* Cliff = GetCliffAt(CurrentIndex))
    {
        PanTargetDistance = FMath::Max(100.f, Cliff->Settings.CliffHeightCm);
    }
    PanCurrentDistance = 0.f;

    FVector Loc = GetActorLocation();
    Loc.Z = PanStartZ;
    SetActorLocation(Loc);

    UpdateHUDInfo();
}

void ACliffSelectionPawn::TickCameraRotate(float DeltaTime)
{
    const FRotator CurrentRot = GetActorRotation();
    const float NewYaw = FMath::FInterpTo(CurrentRot.Yaw, TargetYaw, DeltaTime, CameraInterpSpeed);

    if (!FMath::IsNearlyEqual(NewYaw, CurrentRot.Yaw, 0.01f))
    {
        FRotator NewRot = CurrentRot;
        NewRot.Yaw = NewYaw;
        SetActorRotation(NewRot);
        bIsRotating = true;
    }
    else
    {
        bIsRotating = false;
    }
}

void ACliffSelectionPawn::TickCameraPan(float DeltaTime)
{
    if (CameraPanSpeed <= 0.f || PanTargetDistance <= 0.f)
    {
        return;
    }

    const float MoveDistance = CameraPanSpeed * DeltaTime;
    PanCurrentDistance += MoveDistance;

    FVector Loc = GetActorLocation();
    Loc.Z += MoveDistance;
    SetActorLocation(Loc);

    // ACameraAnimationActor와 동일 - 목표 거리 도달 시 시작 위치로 리셋
    if (PanCurrentDistance >= PanTargetDistance)
    {
        Loc.Z = PanStartZ;
        SetActorLocation(Loc);
        PanCurrentDistance = 0.f;
    }
}

void ACliffSelectionPawn::HandleAllCliffsGenerated()
{
    bHasGeneratedComplete = true;

    // 초기 록온 대상: 인덱스 1 (중앙)
    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    const TArray<float>& Angles = GM ? GM->GetCliffAnglesDeg() : TArray<float>();

    CurrentIndex = Angles.IsValidIndex(1) ? 1 : 0;
    TargetYaw = Angles.IsValidIndex(CurrentIndex) ? Angles[CurrentIndex] : 0.f;

    if (AMountainGenWorldActor* Cliff = GetCliffAt(CurrentIndex))
    {
        PanTargetDistance = FMath::Max(100.f, Cliff->Settings.CliffHeightCm);
    }
    PanCurrentDistance = 0.f;
    PanStartZ = GetActorLocation().Z;

    if (HUDWidget)
    {
        HUDWidget->SetInputHintVisible(true);
    }

    UpdateHUDInfo();
}

void ACliffSelectionPawn::UpdateHUDInfo()
{
    if (!HUDWidget) return;

    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    const TArray<int32>& Seeds = GI ? GI->GetGeneratedSeeds() : TArray<int32>();

    const int32 Seed = Seeds.IsValidIndex(CurrentIndex) ? Seeds[CurrentIndex] : 0;
    const int32 StageIndex = GI ? GI->GetCurrentStageIndex() : 0;

    HUDWidget->UpdateSelectionInfo(CurrentIndex, Seed, GetDifficultyDisplayString(StageIndex));
}

FString ACliffSelectionPawn::GetDifficultyDisplayString(int32 StageIndex)
{
    switch (StageIndex)
    {
    case 2:
        return TEXT("Hard");
    case 1:
    default:
        return TEXT("Medium");
    }
}

AMountainGenWorldActor* ACliffSelectionPawn::GetCliffAt(int32 Index) const
{
    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return nullptr;

    const TArray<AMountainGenWorldActor*>& Cliffs = GM->GetSpawnedCliffs();
    return Cliffs.IsValidIndex(Index) ? Cliffs[Index] : nullptr;
}
