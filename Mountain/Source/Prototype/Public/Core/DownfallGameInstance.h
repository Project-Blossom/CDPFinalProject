#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Core/StageData.h"
#include "DownfallGameInstance.generated.h"

/**
 * 전역 게임 데이터 관리
 * - 스테이지 기록
 * - 세이브/로드
 */
UCLASS()
class PROTOTYPE_API UDownfallGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    // ========== Stage Records ==========

    // 스테이지 시간 기록
    UFUNCTION(BlueprintCallable, Category = "Stage")
    void RecordStageTime(FName StageId, float Time);

    // 스테이지 기록 가져오기
    UFUNCTION(BlueprintPure, Category = "Stage")
    FStageTimeRecord GetStageRecord(FName StageId) const;

    // 모든 스테이지 기록
    UFUNCTION(BlueprintPure, Category = "Stage")
    const TArray<FStageTimeRecord>& GetAllStageRecords() const { return StageRecords; }

    // ========== Save/Load ==========

    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveGame();

    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadGame();

protected:
    // 스테이지 기록 배열
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    TArray<FStageTimeRecord> StageRecords;

    // Save Slot 이름
    UPROPERTY(EditDefaultsOnly, Category = "Save")
    FString SaveSlotName = TEXT("DownfallSave");

    // 스테이지 기록 찾기 (내부 함수)
    FStageTimeRecord* FindStageRecord(FName StageId);
};
