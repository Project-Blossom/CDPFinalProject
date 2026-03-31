#include "Core/DownfallGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/FadeWidget.h"
#include "TimerManager.h"

ADownfallGameMode::ADownfallGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADownfallGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Fade In 효과 시작
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
                
                UE_LOG(LogTemp, Warning, TEXT("Stage Fade In started"));
            }
        }
    }

    // 자동으로 스테이지 시작
    StartStage();
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
