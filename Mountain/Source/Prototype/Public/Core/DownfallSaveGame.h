#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/StageData.h"
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


};
