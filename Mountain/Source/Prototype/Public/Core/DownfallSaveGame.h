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


};
