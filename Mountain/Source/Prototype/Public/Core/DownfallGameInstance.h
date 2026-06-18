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

    // ========== Stage Index ==========

    // 현재 스테이지 인덱스 설정 (1 = Stage_1, 2 = Stage_2, ...)
    // 레벨 로드 시 GameMode 또는 Blueprint에서 호출
    UFUNCTION(BlueprintCallable, Category = "Stage")
    void SetCurrentStageIndex(int32 Index) { CurrentStageIndex = Index; }

    // 현재 스테이지 인덱스 반환
    UFUNCTION(BlueprintPure, Category = "Stage")
    int32 GetCurrentStageIndex() const { return CurrentStageIndex; }

    // ========== Level Transition Flags ==========

    // 다음 스테이지 로드 시 Loading UI 표시 여부
    // CliffSelection → Stage 전환 시 true로 설정, Stage BeginPlay에서 false로 리셋
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetShowLoadingUI(bool bShow) { bShowLoadingUI = bShow; }

    UFUNCTION(BlueprintPure, Category = "UI")
    bool ShouldShowLoadingUI() const { return bShowLoadingUI; }

    // ========== Cliff Selection ==========

    // CliffSelection 레벨 진입 시 호출 — 3개의 새 Seed 생성
    UFUNCTION(BlueprintCallable, Category = "CliffSelection")
    void GenerateNewSeeds();

    // 생성된 Seed 3개 반환
    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    const TArray<int32>& GetGeneratedSeeds() const { return GeneratedSeeds; }

    // 선택된 Seed 설정 (Enter 입력 시 호출)
    UFUNCTION(BlueprintCallable, Category = "CliffSelection")
    void SetSelectedSeed(int32 Seed) { SelectedSeed = Seed; }

    // 선택된 Seed 반환 (다음 스테이지에서 사용)
    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    int32 GetSelectedSeed() const { return SelectedSeed; }

protected:
    // 스테이지 기록 배열
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    TArray<FStageTimeRecord> StageRecords;

    // 현재 활성 세이브 슬롯 (0, 1, 2)
    UPROPERTY(BlueprintReadOnly, Category = "Save")
    int32 CurrentSaveSlotIndex = 0;

    // NewGame 모드 플래그
    bool bIsNewGameMode = false;

    // 레벨 전환 시 Loading UI 표시 여부
    UPROPERTY(BlueprintReadOnly, Category = "UI")
    bool bShowLoadingUI = false;

    // 현재 스테이지 인덱스 (1-based, 0 = 미설정)
    // VFX 분기, 난이도 조정 등에 활용
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    int32 CurrentStageIndex = 0;

    // 세이브 슬롯 이름 생성
    FString GetSlotName(int32 SlotIndex) const;

    // 스테이지 기록 찾기 (내부 함수)
    FStageTimeRecord* FindStageRecord(FName StageId);

    // ========== Cliff Selection (Runtime Only) ==========

    // CliffSelection 레벨에서 생성된 Seed 3개 (스테이지 전환 간 임시 데이터, 세이브 안 됨)
    UPROPERTY(BlueprintReadOnly, Category = "CliffSelection")
    TArray<int32> GeneratedSeeds;

    // 플레이어가 선택(Enter)한 Seed — 다음 스테이지 레벨 로드 시 사용
    UPROPERTY(BlueprintReadOnly, Category = "CliffSelection")
    int32 SelectedSeed = 0;
};
