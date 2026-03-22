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

    // Fade Out 시간
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    float FadeOutDuration = 2.0f;

    // Fade Out Post Process Material
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    TObjectPtr<class UMaterialInterface> FadeOutMaterial;

    // Fade In Widget 클래스 (게임 시작 시)
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    TSubclassOf<class UFadeWidget> FadeInWidgetClass;

    // Fade In 시간
    UPROPERTY(EditDefaultsOnly, Category = "Stage")
    float FadeInDuration = 1.0f;

private:
    bool bStageCompleted = false;
    bool bFadingOut = false;
    FTimerHandle FadeOutTimerHandle;
    
    // Post Process Material Instance
    UPROPERTY()
    TObjectPtr<class UMaterialInstanceDynamic> FadeOutMaterialInstance;
    
    // Fade 진행도
    float CurrentFadeAlpha = 0.0f;

    void StartFadeOut();
    void UpdateFadeOut(float DeltaTime);
    void OnFadeOutComplete();

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
