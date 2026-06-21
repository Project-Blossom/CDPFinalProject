#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/StageData.h"
#include "MountainGenSettings.h"
#include "DownfallSaveGame.generated.h"

/**
 * 세이브 데이터
 */
UCLASS()
class PROTOTYPE_API UDownfallSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // 스테이지 기록
    UPROPERTY()
    TArray<FStageTimeRecord> StageRecords;

    // 총 플레이 시간 (초)
    UPROPERTY()
    float TotalPlayTime = 0.0f;

    // 마지막 저장 시간
    UPROPERTY()
    FDateTime LastSaveTime;

    // 클리어한 스테이지 개수
    UPROPERTY()
    int32 ClearedStagesCount = 0;

    // [NEW] LoadGame 진행상황 동기화용 — 저장 시점의 진행 상태를 그대로 복원하기 위한 값.
    // CurrentStageIndex: 현재(또는 가장 최근에) 플레이 중이던 스테이지 번호(1/2/3).
    // CliffSelection 확정 시점에 이미 그 스테이지로 갱신되므로(아직 클리어 전이어도),
    // "이 스테이지를 클리어했는지(StageRecords[N].bCleared)"와 같이 보면 정확한 재진입
    // 지점을 계산할 수 있다.
    UPROPERTY()
    int32 SavedCurrentStageIndex = 0;

    // CurrentStageIndex 스테이지에 대해 CliffSelection에서 확정한 Seed/Difficulty.
    // 그 스테이지를 클리어하기 전에 LoadGame하면, CliffSelection을 다시 거치지 않고
    // 동일한 암벽으로 바로 재진입하는 데 사용한다.
    UPROPERTY()
    int32 SavedSelectedSeed = 0;

    UPROPERTY()
    EMountainGenDifficulty SavedSelectedDifficulty = EMountainGenDifficulty::Normal;
};
