#include "Core/DownfallGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/PlayerCameraManager.h"

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

    // Fade Out 중이면 업데이트
    if (bFadingOut)
    {
        UpdateFadeOut(DeltaTime);
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

    if (!FadeOutMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("FadeOutMaterial not set! Skipping fade effect."));
        OnFadeOutComplete();
        return;
    }

    // Material Instance Dynamic 생성
    FadeOutMaterialInstance = UMaterialInstanceDynamic::Create(FadeOutMaterial, this);
    
    if (!FadeOutMaterialInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FadeOutMaterialInstance!"));
        OnFadeOutComplete();
        return;
    }

    // PlayerCameraManager에 Post Process Material 추가
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->PlayerCameraManager)
    {
        // PostProcessSettings에 Material 추가
        FPostProcessSettings PPSettings;
        FWeightedBlendable Blendable(1.0f, FadeOutMaterialInstance);
        PPSettings.WeightedBlendables.Array.Add(Blendable);
        
        PC->PlayerCameraManager->AddCachedPPBlend(PPSettings, 1.0f);
        CurrentFadeAlpha = 0.0f;
        bFadingOut = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Fade Out Post Process applied!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerCameraManager not found!"));
        OnFadeOutComplete();
    }
}

void ADownfallGameMode::UpdateFadeOut(float DeltaTime)
{
    if (!FadeOutMaterialInstance)
        return;

    // Alpha 값 증가 (0 → 1)
    CurrentFadeAlpha += DeltaTime / FadeOutDuration;
    CurrentFadeAlpha = FMath::Clamp(CurrentFadeAlpha, 0.0f, 1.0f);

    // Material Parameter 업데이트
    FadeOutMaterialInstance->SetScalarParameterValue(FName("FadeAlpha"), CurrentFadeAlpha);
    
    UE_LOG(LogTemp, Log, TEXT("Fade Alpha: %.2f"), CurrentFadeAlpha);

    // Fade 완료 체크
    if (CurrentFadeAlpha >= 1.0f)
    {
        bFadingOut = false;
        OnFadeOutComplete();
    }
}

void ADownfallGameMode::OnFadeOutComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Fade Out Complete - Loading Result Level: %s"), 
        *ResultLevelName.ToString());

    // 결과 화면 레벨로 전환
    UGameplayStatics::OpenLevel(this, ResultLevelName);
}
