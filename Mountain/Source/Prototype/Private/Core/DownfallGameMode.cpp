#include "Core/DownfallGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/FadeWidget.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "MountainGenWorldActor.h"
#include "EngineUtils.h"

ADownfallGameMode::ADownfallGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADownfallGameMode::BeginPlay()
{
    Super::BeginPlay();

    UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());

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
        bool bFoundActor = false;

        for (TActorIterator<AMountainGenWorldActor> It(GetWorld()); It; ++It)
        {
            AMountainGenWorldActor* MountainActor = *It;
            if (MountainActor)
            {
                MountainActor->Settings.Seed = SelectedSeed;
                MountainActor->OnMountainGenerated.AddDynamic(
                    this, &ADownfallGameMode::OnCliffGenerationComplete);
                MountainActor->Regenerate();
                bFoundActor = true;

                UE_LOG(LogTemp, Warning, TEXT("StageGameMode: MountainGenWorldActor Seed=%d → Regenerate()"), SelectedSeed);
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

    HideLoadingWidget();
    StartStageAfterLoading();
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
