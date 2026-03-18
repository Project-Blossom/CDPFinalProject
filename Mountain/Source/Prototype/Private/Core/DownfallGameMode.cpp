#include "Core/DownfallGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "Kismet/GameplayStatics.h"

ADownfallGameMode::ADownfallGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADownfallGameMode::BeginPlay()
{
    Super::BeginPlay();

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

    bStageActive = false;
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
}

float ADownfallGameMode::GetCurrentElapsedTime() const
{
    if (bStageActive)
    {
        return GetWorld()->GetTimeSeconds() - StageStartTime;
    }
    return CurrentElapsedTime;
}
