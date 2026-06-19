// File: Source/Prototype/Private/Core/CliffSelectionPawn.cpp
#include "Core/CliffSelectionPawn.h"
#include "Core/CliffSelectionGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "UI/CliffSelectionHUDWidget.h"
#include "UI/CliffSelectionLoadingWidget.h"
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
#include "GameFramework/PlayerStart.h"

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
    // PanStartZ: 항상 0 기준 (PlayerStart Z=0, 암벽 하단에서 상단까지 패닝)
    PanStartZ = 0.f;
    PanCurrentDistance = 0.f;

    // 이전 레벨(StageResult 등)에서 UIOnly 입력 모드로 전환된 채로 넘어온 경우를 대비해
    // 게임 입력이 정상 작동하도록 GameOnly로 복구
    if (APlayerController* InputPC = Cast<APlayerController>(GetController()))
    {
        FInputModeGameOnly GameOnlyMode;
        InputPC->SetInputMode(GameOnlyMode);
        InputPC->bShowMouseCursor = false;
    }

    // 로딩 위젯 생성 (Pawn BeginPlay 시점 = PC 확보 후이므로 안전)
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (LoadingWidgetClass)
        {
            LoadingWidget = CreateWidget<UCliffSelectionLoadingWidget>(PC, LoadingWidgetClass);
            if (LoadingWidget)
            {
                LoadingWidget->AddToViewport(10); // HUD보다 위에
            }
        }
    }

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
        if (RerollAction)
        {
            EIC->BindAction(RerollAction, ETriggerEvent::Started, this, &ACliffSelectionPawn::OnRerollPressed);
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
    GI->SetShowLoadingUI(true);  // 다음 스테이지 BeginPlay에서 Loading UI 표시

    UE_LOG(LogTemp, Warning, TEXT("CliffSelectionPawn: Confirmed Seed=%d Index=%d → NextStage=%s"),
        Seeds[CurrentIndex], CurrentIndex, *NextStageLevelName.ToString());

    if (NextStageLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionPawn: NextStageLevelName not set"));
        return;
    }

    UGameplayStatics::OpenLevel(this, NextStageLevelName);
}

void ACliffSelectionPawn::OnRerollPressed(const FInputActionValue& Value)
{
    if (!bHasGeneratedComplete) return;

    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    UE_LOG(LogTemp, Warning, TEXT("CliffSelectionPawn: Reroll requested"));

    // 리롤 완료 시 GameMode가 OnAllCliffsGenerated를 다시 브로드캐스트하므로
    // HandleAllCliffsGenerated에서 초기 록온/HUD가 자동으로 갱신됨
    bHasGeneratedComplete = false;
    GM->RerollCliffs();
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

    // 해당 암벽 정면 수직 CameraDistanceCm 위치로 XY 이동
    SetCameraPositionForCliff(CurrentIndex);

    // 패닝 타겟 거리 갱신 (새로 록온된 암벽의 CliffHeightCm)
    // Z 진행도(PanCurrentDistance)는 유지 — 슬라이드 연속성 보장
    if (AMountainGenWorldActor* Cliff = GetCliffAt(CurrentIndex))
    {
        PanTargetDistance = FMath::Max(100.f, Cliff->Settings.CliffHeightCm);
    }

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

    // XY 위치도 동일한 보간 속도로 부드럽게 이동
    FVector CurrentLoc = GetActorLocation();
    const float NewX = FMath::FInterpTo(CurrentLoc.X, TargetXY.X, DeltaTime, CameraInterpSpeed);
    const float NewY = FMath::FInterpTo(CurrentLoc.Y, TargetXY.Y, DeltaTime, CameraInterpSpeed);

    if (!FMath::IsNearlyEqual(NewX, CurrentLoc.X, 0.1f) || !FMath::IsNearlyEqual(NewY, CurrentLoc.Y, 0.1f))
    {
        CurrentLoc.X = NewX;
        CurrentLoc.Y = NewY;
        SetActorLocation(CurrentLoc);
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

    // PanStartZ: 항상 0 기준 (PlayerStart Z=0, 암벽 하단에서 상단까지 패닝)
    PanStartZ = 0.f;
    SetCameraPositionForCliff(CurrentIndex);
    FVector InitLoc = GetActorLocation();
    InitLoc.X = TargetXY.X;
    InitLoc.Y = TargetXY.Y;
    InitLoc.Z = 0.f; // Z 명시적으로 초기화
    SetActorLocation(InitLoc);

    if (AMountainGenWorldActor* Cliff = GetCliffAt(CurrentIndex))
    {
        PanTargetDistance = FMath::Max(100.f, Cliff->Settings.CliffHeightCm);
    }
    PanCurrentDistance = 0.f;

    if (HUDWidget)
    {
        HUDWidget->SetInputHintVisible(true);
    }

    UpdateHUDInfo();
}

void ACliffSelectionPawn::SetCameraPositionForCliff(int32 Index)
{
    AMountainGenWorldActor* Cliff = GetCliffAt(Index);
    if (!Cliff) return;

    const FVector CliffLocation = Cliff->GetActorLocation();
    const FVector CliffForward = Cliff->GetActorForwardVector();

    // 암벽 정면에서 카메라 위치: 암벽으로부터 정면 방향으로 Distance만큼
    const FVector Target = CliffLocation + CliffForward * CameraDistanceCm;

    // 목표 XY만 설정 — 실제 이동은 TickCameraRotate에서 FInterpTo로 처리
    TargetXY.X = Target.X;
    TargetXY.Y = Target.Y;
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
