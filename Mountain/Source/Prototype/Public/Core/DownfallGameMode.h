#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/StageData.h"
#include "DownfallGameMode.generated.h"

/**
 * 스테이지 플레이 관리 GameMode
 * - 시간 기록
 * - 클리어 조건 체크
 */
UCLASS()
class PROTOTYPE_API ADownfallGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADownfallGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // 현재 스테이지 ID (Blueprint에서 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    FName CurrentStageId = NAME_None;

    // 스테이지 시작 (자동 호출됨)
    UFUNCTION(BlueprintCallable, Category = "Stage")
    void StartStage();

    // 스테이지 클리어
    UFUNCTION(BlueprintCallable, Category = "Stage")
    void CompleteStage();

    // 현재 경과 시간 가져오기
    UFUNCTION(BlueprintPure, Category = "Stage")
    float GetCurrentElapsedTime() const;

    // 스테이지 진행 중인지
    UFUNCTION(BlueprintPure, Category = "Stage")
    bool IsStageActive() const { return bStageActive; }

protected:
    // 결과 화면 레벨 이름
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    FName ResultLevelName = FName("StageResult");

    // Fade Out Widget 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    TSubclassOf<class UFadeWidget> FadeOutWidgetClass;

    // Fade Out 시간
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    float FadeOutDuration = 2.0f;

    // Fade In Widget 클래스 (게임 시작 시)
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    TSubclassOf<class UFadeWidget> FadeInWidgetClass;

    // Fade In 시간
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    float FadeInDuration = 1.0f;

    // 로딩 UI 위젯 클래스 (WBP_CliffSelectionLoading 재사용 또는 별도 위젯)
    UPROPERTY(EditDefaultsOnly, Category = "Stage|Loading")
    TSubclassOf<class UUserWidget> LoadingWidgetClass;

private:
    bool bStageCompleted = false;
    
    // Fade Out Widget Instance
    UPROPERTY()
    TObjectPtr<class UFadeWidget> FadeOutWidget;

    // Loading Widget Instance
    UPROPERTY()
    TObjectPtr<class UUserWidget> LoadingWidgetInstance;

    void StartFadeOut();
    void OnFadeOutComplete();

    // 암벽 생성 완료 콜백 — Loading UI 제거 후 스테이지 시작
    UFUNCTION()
    void OnCliffGenerationComplete(AActor* Generator);

    // Loading UI 제거
    void HideLoadingWidget();

    // Fade In + StartStage 묶음
    void StartStageAfterLoading();

    // [DEBUG-FIX] 미니맵 캡처(셰이더 워밍업 대기 포함)가 끝난 뒤에 로딩 화면을 내리기 위한 타이머.
    // ADownfallCharacter::AutoConfigureMinimapCapture()는 머티리얼 교체까지만 동기로 처리하고
    // 실제 CaptureScene()은 MinimapCaptureMaterialWarmupDelay초 뒤 FinalizeMinimapCapture()에서
    // 수행하므로, 로딩 화면 해제도 그 시점 이후로 맞춰야 "암벽 생성 → 미니맵 머티리얼 적용 →
    // 캡처 → 복구 → 로딩 화면 해제" 순서가 실제로 보장된다.
    FTimerHandle MinimapCaptureLoadingDelayHandle;

    void FinishLoadingAfterMinimapCapture();

protected:
    // 스테이지 활성화 상태
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    bool bStageActive = false;

    // 스테이지 시작 시간
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    float StageStartTime = 0.0f;

    // 현재 경과 시간
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    float CurrentElapsedTime = 0.0f;
};
