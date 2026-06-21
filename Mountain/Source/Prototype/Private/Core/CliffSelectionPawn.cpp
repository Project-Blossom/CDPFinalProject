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

    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->PlayMenuBGM(this, GI->CliffSelectionBGM);
    }

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

    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->PlayUISound(this, GI->CliffSelectionMoveLeftSound);
    }

    StartCameraRotateTo(CurrentIndex - 1);
}

void ACliffSelectionPawn::OnMoveSelectionRight(const FInputActionValue& Value)
{
    if (!bHasGeneratedComplete) return;

    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    const int32 MaxIndex = GM ? (GM->GetSpawnedCliffs().Num() - 1) : 2;

    if (CurrentIndex >= MaxIndex) return;

    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->PlayUISound(this, GI->CliffSelectionMoveRightSound);
    }

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

    // Enter 확정음은 레벨 전환 뒤에도 끝까지 들려야 하므로 persistent UI 채널로 재생한다.
    GI->PlayPersistentUISound(GI->CliffSelectionConfirmSound);
    GI->StopMenuBGM(0.0f);

    GI->SetSelectedSeed(Seeds[CurrentIndex]);
    const int32 NextStageIndex = GI->GetCurrentStageIndex() + 1;
    GI->SetCurrentStageIndex(NextStageIndex);
    GI->SetShowLoadingUI(true);  // 다음 스테이지 BeginPlay에서 Loading UI 표시

    // [NEW] LoadGame 진행상황 동기화.
    // 여기서 곧바로 SaveGame()을 호출해, "암벽 선택까지는 끝냈지만 그 스테이지를 아직
    // 클리어하지 못한" 상태를 디스크에 즉시 기록한다. 이렇게 해두면 이 시점 이후
    // 스테이지를 클리어하지 않고 종료하더라도, 다음에 LoadGame하면 CliffSelection을
    // 다시 거치지 않고 방금 선택한 것과 동일한 Seed/Difficulty로 이 스테이지에 바로
    // 재진입한다(GetResumeLevelName() 참고).
    GI->SaveGame();

    // CurrentStageIndex 기준으로 다음 레벨을 동적으로 결정.
    // NextStageLevelName(BP 고정값)을 그대로 쓰면 CliffSelection 레벨을 여러 스테이지가
    // 공유하는 구조상 항상 같은 레벨로만 이동하는 문제가 생기므로 StageIndex 기반으로 계산한다.
    FName ResolvedNextLevel;
    if (NextStageIndex == 2)
    {
        ResolvedNextLevel = FName("Stage_2");
    }
    else if (NextStageIndex == 3)
    {
        ResolvedNextLevel = FName("Stage_3");
    }
    else
    {
        // Ending 미구현 — 현재는 Stage_3 유지. BP에서 NextStageLevelName을 별도로
        // 지정해둔 경우 그 값을 우선 사용할 수 있도록 폴백 처리.
        ResolvedNextLevel = NextStageLevelName.IsNone() ? FName("Stage_3") : NextStageLevelName;
        UE_LOG(LogTemp, Warning, TEXT("CliffSelectionPawn: StageIndex=%d, Ending not implemented yet -> %s"),
            NextStageIndex, *ResolvedNextLevel.ToString());
    }

    UE_LOG(LogTemp, Warning, TEXT("CliffSelectionPawn: Confirmed Seed=%d Index=%d → NextStage=%s [StageIndex=%d]"),
        Seeds[CurrentIndex], CurrentIndex, *ResolvedNextLevel.ToString(), NextStageIndex);

    UGameplayStatics::OpenLevel(this, ResolvedNextLevel);
}

void ACliffSelectionPawn::OnRerollPressed(const FInputActionValue& Value)
{
    if (!bHasGeneratedComplete) return;

    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return;

    UE_LOG(LogTemp, Warning, TEXT("CliffSelectionPawn: Reroll requested"));

    if (UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>())
    {
        GI->PlayUISound(this, GI->CliffSelectionRerollSound);
    }

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

    // [DEBUG-FIX] HUD<->실제 지형 시드/난이도 불일치 버그 수정.
    // 기존에는 GI->GetGeneratedSeeds()[CurrentIndex](=AutoTune 탐색 전 "요청 시드")와
    // StageIndex로부터 별도로 재계산한 난이도 문자열을 표시했다. 이 값들은 실제로
    // 화면에 보이는 절벽을 만든 SeedSearch 결과(FinalSeed)나 ResolveDifficultyFromStageIndex()가
    // 실제로 선택한 Difficulty와 다를 수 있어, HUD에 표시되는 정보와 실제 지형이 어긋났다.
    // 대신 실제로 생성이 완료된 Cliff 액터의 Settings를 그대로 읽어 단일 진실 공급원으로 삼는다.
    // 이 값은 CliffSelectionGameMode::SpawnCliffs()에서 ApplyGeneratedMeshResult 이후
    // 확정된 FinalSeed/Difficulty이며, Stage 진입 시 동일한 InputSeed+Difficulty로
    // 재탐색했을 때도 결정론적으로 같은 FinalSeed가 나오므로 Stage에서 실제로 플레이하는
    // 지형과도 일치한다.
    AMountainGenWorldActor* Cliff = GetCliffAt(CurrentIndex);

    const int32 Seed = Cliff ? Cliff->Settings.Seed : 0;
    const EMountainGenDifficulty Difficulty = Cliff ? Cliff->Settings.Difficulty : EMountainGenDifficulty::Normal;

    HUDWidget->UpdateSelectionInfo(CurrentIndex, Seed, GetDifficultyDisplayString(Difficulty));
}

FString ACliffSelectionPawn::GetDifficultyDisplayString(EMountainGenDifficulty Difficulty)
{
    switch (Difficulty)
    {
    case EMountainGenDifficulty::Hard:
        return TEXT("Hard");
    case EMountainGenDifficulty::Normal:
        return TEXT("Medium");
    case EMountainGenDifficulty::Easy:
    default:
        return TEXT("Easy");
    }
}

AMountainGenWorldActor* ACliffSelectionPawn::GetCliffAt(int32 Index) const
{
    ACliffSelectionGameMode* GM = Cast<ACliffSelectionGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) return nullptr;

    const TArray<AMountainGenWorldActor*>& Cliffs = GM->GetSpawnedCliffs();
    return Cliffs.IsValidIndex(Index) ? Cliffs[Index] : nullptr;
}