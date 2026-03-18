#pragma once

#include "CoreMinimal.h"
#include "StageData.generated.h"

/**
 * 스테이지 클리어 기록
 */
USTRUCT(BlueprintType)
struct FStageTimeRecord
{
    GENERATED_BODY()

    // 스테이지 고유 ID (Stage1)
    UPROPERTY(BlueprintReadWrite)
    FName StageId;

    // 최고 기록 (초)
    UPROPERTY(BlueprintReadWrite)
    float BestTime = 0.0f;

    // 최근 플레이 시간 (초)
    UPROPERTY(BlueprintReadWrite)
    float LastTime = 0.0f;

    // 클리어 횟수
    UPROPERTY(BlueprintReadWrite)
    int32 ClearCount = 0;

    // 클리어 여부
    UPROPERTY(BlueprintReadWrite)
    bool bCleared = false;

    FStageTimeRecord()
        : StageId(NAME_None)
        , BestTime(0.0f)
        , LastTime(0.0f)
        , ClearCount(0)
        , bCleared(false)
    {}

    FStageTimeRecord(FName InStageId)
        : StageId(InStageId)
        , BestTime(0.0f)
        , LastTime(0.0f)
        , ClearCount(0)
        , bCleared(false)
    {}
};

/*
  스테이지 메타데이터
 */
USTRUCT(BlueprintType)
struct FStageMetadata
{
    GENERATED_BODY()

    // 스테이지 고유 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StageId;

    // 표시 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    // 목표 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetTime = 60.0f;

    // 레벨 경로
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UWorld> LevelAsset;

    FStageMetadata()
        : StageId(NAME_None)
        , DisplayName(FText::FromString(TEXT("Unknown Stage")))
        , TargetTime(60.0f)
    {}
};
