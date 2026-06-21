#include "Core/DownfallGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/FadeWidget.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "MountainGenWorldActor.h"
#include "EngineUtils.h"
#include "DownfallCharacter.h"

ADownfallGameMode::ADownfallGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADownfallGameMode::BeginPlay()
{
    Super::BeginPlay();

    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());

    // 실제 스테이지 월드가 시작되는 순간 메뉴/결과/암벽선택 BGM은 남아 있으면 안 된다.
    if (GI)
    {
        GI->StopMenuBGM(0.0f);
    }

    // ── Loading UI 처리 ──────────────────────────────────────
    // CliffSelection에서 넘어온 경우 Loading UI 표시 후 암벽 생성 완료 시 제거
    if (GI && GI->ShouldShowLoadingUI() && LoadingWidgetClass)
    {
        APlayerController* LPPC = UGameplayStatics::GetPlayerController(this, 0);
        if (LPPC)
        {
            LoadingWidgetInstance = CreateWidget<UUserWidget>(LPPC, LoadingWidgetClass);
            if (LoadingWidgetInstance)
            {
                LoadingWidgetInstance->AddToViewport(100);
                UE_LOG(LogTemp, Warning, TEXT("StageGameMode: Loading UI displayed"));
            }
        }
        GI->SetShowLoadingUI(false);
    }

    // ── Seed 동기화 ───────────────────────────────────────────
    // GameInstance에 저장된 SelectedSeed로 레벨의 MountainGenWorldActor 재생성
    if (GI && GI->GetSelectedSeed() > 0)
    {
        const int32 SelectedSeed = GI->GetSelectedSeed();

        // [DEBUG-FIX] CliffSelection<->Stage 시드 드리프트 버그 수정.
        // BuildChunkAndMesh()는 항상 MGApplyDifficultyPreset(S)를 거치며, 이 함수는
        // Difficulty 값에 따라 Targets/BaseField3DStrengthCm/OverhangBias 등 시드 탐색
        // 스코어링에 직접 관여하는 필드를 전부 덮어쓴다. 레벨에 배치된 MountainGenWorldActor의
        // Settings.Difficulty(에디터에서 설정한 값, 기본 Easy)가 CliffSelection이 실제로
        // 시드를 탐색할 때 사용한 Difficulty(Normal/Hard)와 다르면, 같은 Seed를 넣어도
        // MGSearchSeedForTargets가 완전히 다른 후보를 "최적"으로 선택해 FinalSeed가
        // CliffSelection에서 본 절벽과 달라진다. Seed뿐 아니라 Difficulty도 GameInstance
        // 값으로 강제 동기화해 레벨 에디터 설정값에 의존하지 않도록 한다.
        const EMountainGenDifficulty SelectedDifficulty = GI->GetSelectedDifficulty();

        bool bFoundActor = false;

        for (TActorIterator<AMountainGenWorldActor> It(GetWorld()); It; ++It)
        {
            AMountainGenWorldActor* MountainActor = *It;
            if (MountainActor)
            {
                MountainActor->Settings.Seed = SelectedSeed;
                MountainActor->Settings.Difficulty = SelectedDifficulty;
                MountainActor->OnMountainGenerated.AddDynamic(
                    this, &ADownfallGameMode::OnCliffGenerationComplete);
                MountainActor->Regenerate();
                bFoundActor = true;

                UE_LOG(LogTemp, Warning, TEXT("StageGameMode: MountainGenWorldActor Seed=%d Difficulty=%d → Regenerate()"),
                    SelectedSeed, (int32)SelectedDifficulty);
                break;
            }
        }

        if (!bFoundActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("StageGameMode: MountainGenWorldActor not found, skipping Seed sync"));
            HideLoadingWidget();
            StartStageAfterLoading();
        }
    }
    else
    {
        // Seed가 없으면 (직접 레벨 진입 등) 즉시 시작
        HideLoadingWidget();
        StartStageAfterLoading();
    }
}

void ADownfallGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 스테이지 활성화 중이면 시간 업데이트
    if (bStageActive)
    {
        CurrentElapsedTime = GetWorld()->GetTimeSeconds() - StageStartTime;
    }
}

void ADownfallGameMode::StartStage()
{
    if (bStageActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stage already active!"));
        return;
    }

    bStageActive = true;
    StageStartTime = GetWorld()->GetTimeSeconds();
    CurrentElapsedTime = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("=== Stage Started: %s ==="), *CurrentStageId.ToString());
}

void ADownfallGameMode::CompleteStage()
{
    if (!bStageActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stage not active!"));
        return;
    }

    if (bStageCompleted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stage already completed!"));
        return;
    }

    bStageActive = false;
    bStageCompleted = true;
    float FinalTime = GetCurrentElapsedTime();

    UE_LOG(LogTemp, Warning, TEXT("=== Stage Completed: %s, Time: %.2f seconds ==="),
        *CurrentStageId.ToString(), FinalTime);

    // GameInstance에 기록 저장
    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->RecordStageTime(CurrentStageId, FinalTime);
        GI->SetLastClearedStageId(CurrentStageId);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DownfallGameInstance not found!"));
    }

    // Fade Out 시작
    StartFadeOut();
}

float ADownfallGameMode::GetCurrentElapsedTime() const
{
    if (bStageActive)
    {
        return GetWorld()->GetTimeSeconds() - StageStartTime;
    }
    return CurrentElapsedTime;
}

void ADownfallGameMode::StartFadeOut()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting Fade Out (%.1f seconds)..."), FadeOutDuration);

    if (!FadeOutWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("FadeOutWidgetClass not set! Skipping fade effect."));
        OnFadeOutComplete();
        return;
    }

    // Widget 생성
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController not found!"));
        OnFadeOutComplete();
        return;
    }

    FadeOutWidget = CreateWidget<UFadeWidget>(PC, FadeOutWidgetClass);
    if (!FadeOutWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FadeOutWidget!"));
        OnFadeOutComplete();
        return;
    }

    // Widget을 최상위에 추가
    FadeOutWidget->AddToViewport(200);  // High Z-Order

    // Fade Out 시작
    FadeOutWidget->StartFadeOut(FadeOutDuration);

    UE_LOG(LogTemp, Warning, TEXT("Fade Out Widget started!"));

    // FadeOutDuration 후 레벨 전환
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        this,
        &ADownfallGameMode::OnFadeOutComplete,
        FadeOutDuration,
        false
    );
}

void ADownfallGameMode::OnFadeOutComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Fade Out Complete - Loading Result Level: %s"),
        *ResultLevelName.ToString());

    // 결과 화면 레벨로 전환
    UGameplayStatics::OpenLevel(this, ResultLevelName);
}

void ADownfallGameMode::OnCliffGenerationComplete(AActor* Generator)
{
    // 암벽 생성 완료 → Loading UI 제거 후 스테이지 시작
    UE_LOG(LogTemp, Warning, TEXT("StageGameMode: Cliff generation complete, hiding loading UI"));

    // [DEBUG] CliffSelection<->Stage Seed 동기화 검증용. GI->GetSelectedSeed()는
    // CliffSelection에서 확정한 요청 Seed, MountainActor->Settings.Seed는
    // 이 콜백 시점에 실제로 적용된(SeedSearch 이후일 수 있는) 최종 Seed.
    {
        UDownfallGameInstance* GIDebug = Cast<UDownfallGameInstance>(GetGameInstance());
        AMountainGenWorldActor* MountainActorDebug = Cast<AMountainGenWorldActor>(Generator);
        const int32 RequestedSeed = GIDebug ? GIDebug->GetSelectedSeed() : -1;
        const int32 ActualSeed = MountainActorDebug ? MountainActorDebug->Settings.Seed : -1;
        UE_LOG(LogTemp, Warning, TEXT("StageGameMode: SeedCheck RequestedSeed(GI)=%d ActualSeed(Mountain)=%d %s"),
            RequestedSeed, ActualSeed,
            (RequestedSeed == ActualSeed) ? TEXT("MATCH") : TEXT("MISMATCH"));
    }

    HideLoadingWidget();
    StartStageAfterLoading();

    // [DEBUG-FIX] 미니맵 캡처 타이밍 버그 수정.
    // ADownfallCharacter::BeginPlay()는 AutoConfigureMinimapCapture()를 한 번 호출하지만,
    // 그 시점에는 MountainActor->Regenerate()가 비동기로 진행 중이라 ProcMesh 바운드가
    // 아직 새 Seed로 갱신되지 않은 상태(레벨 배치 시점의 프리뷰 메시 바운드)다.
    // AutoConfigureMinimapCapture()는 그 바운드로 SceneCapture2D 위치/OrthoWidth를
    // 계산하므로, 결과적으로 미니맵이 엉뚱한 범위를 캡처하거나(스테이지마다 다른 절벽
    // 크기) 잘못 정렬된 채로 멈춰 있게 된다.
    // 메시 생성이 실제로 완료된 이 콜백(OnMountainGenerated) 시점에 플레이어 캐릭터의
    // AutoConfigureMinimapCapture()를 다시 호출해, 최종 ProcMesh 바운드 기준으로
    // SceneCapture를 재설정하고 RT_Minimap을 다시 캡처한다.
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
    {
        if (ADownfallCharacter* PlayerCharacter = Cast<ADownfallCharacter>(PlayerPawn))
        {
            PlayerCharacter->AutoConfigureMinimapCapture();
            UE_LOG(LogTemp, Warning, TEXT("StageGameMode: AutoConfigureMinimapCapture() re-triggered after mesh regen complete"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("StageGameMode: PlayerPawn is not ADownfallCharacter, skipping minimap reconfigure"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("StageGameMode: PlayerPawn not found, skipping minimap reconfigure"));
    }
}

void ADownfallGameMode::HideLoadingWidget()
{
    if (LoadingWidgetInstance)
    {
        LoadingWidgetInstance->RemoveFromParent();
        LoadingWidgetInstance = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("StageGameMode: Loading UI hidden"));
    }
}

void ADownfallGameMode::StartStageAfterLoading()
{
    // Fade In 효과
    if (FadeInWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            UFadeWidget* FadeWidget = CreateWidget<UFadeWidget>(PC, FadeInWidgetClass);
            if (FadeWidget)
            {
                FadeWidget->AddToViewport(200);
                FadeWidget->StartFadeIn(FadeInDuration);
                UE_LOG(LogTemp, Warning, TEXT("StageGameMode: Fade In started"));
            }
        }
    }

    // 스테이지 시작
    StartStage();
}