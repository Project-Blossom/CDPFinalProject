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
