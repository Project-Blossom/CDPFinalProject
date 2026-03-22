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

    // 현재 활성 슬롯 설정
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SetCurrentSaveSlot(int32 SlotIndex);

    // 현재 슬롯 가져오기
    UFUNCTION(BlueprintPure, Category = "Save")
    int32 GetCurrentSaveSlot() const { return CurrentSaveSlotIndex; }

    // 특정 슬롯 저장
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveToSlot(int32 SlotIndex);

    // 특정 슬롯 로드
    UFUNCTION(BlueprintCallable, Category = "Save")
    bool LoadFromSlot(int32 SlotIndex);

    // 현재 슬롯 저장
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveGame();

    // 현재 슬롯 로드
    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadGame();

    // 슬롯 삭제 (초기화)
    UFUNCTION(BlueprintCallable, Category = "Save")
    void DeleteSlot(int32 SlotIndex);

    // 슬롯 존재 여부 확인
    UFUNCTION(BlueprintPure, Category = "Save")
    bool DoesSaveSlotExist(int32 SlotIndex) const;

    // 슬롯 정보 가져오기 (플레이 시간, 클리어 개수, 마지막 날짜)
    UFUNCTION(BlueprintPure, Category = "Save")
    void GetSlotInfo(int32 SlotIndex, float& OutTotalPlayTime, int32& OutClearedStages, FDateTime& OutLastSaveTime, bool& OutExists) const;

    // NewGame 모드 설정
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SetNewGameMode(bool bNewGame) { bIsNewGameMode = bNewGame; }

    // NewGame 모드인지 확인
    UFUNCTION(BlueprintPure, Category = "Save")
    bool IsNewGameMode() const { return bIsNewGameMode; }

protected:
    // 스테이지 기록 배열
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    TArray<FStageTimeRecord> StageRecords;

    // 현재 활성 세이브 슬롯 (0, 1, 2)
    UPROPERTY(BlueprintReadOnly, Category = "Save")
    int32 CurrentSaveSlotIndex = 0;

    // NewGame 모드 플래그
    bool bIsNewGameMode = false;

    // 세이브 슬롯 이름 생성
    FString GetSlotName(int32 SlotIndex) const;

    // 스테이지 기록 찾기 (내부 함수)
    FStageTimeRecord* FindStageRecord(FName StageId);
};
