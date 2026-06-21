#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Core/StageData.h"
#include "MountainGenSettings.h"
#include "DownfallGameInstance.generated.h"

class USoundBase;
class UAudioComponent;

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

    // [NEW] 특정 슬롯의 스테이지별 기록(StageRecords)만 조회한다.
    // LoadFromSlot()과 달리 CurrentSaveSlotIndex/StageRecords(현재 활성 슬롯 상태)는
    // 전혀 건드리지 않는다 — Ending 결과 화면에서 "3개 슬롯 통틀어 베스트"를 계산하려면
    // 현재 플레이 중인 슬롯 상태를 유지한 채로 다른 슬롯들의 기록도 들여다봐야 하기 때문.
    // 슬롯이 없거나 비어 있으면 빈 배열을 반환한다.
    UFUNCTION(BlueprintPure, Category = "Save")
    TArray<FStageTimeRecord> GetStageRecordsFromSlot(int32 SlotIndex) const;

    // [NEW] Ending 결과 화면 전용 — 특정 StageId의 "3개 SaveSlot 통틀어 가장 짧은
    // BestTime"을 반환한다. 기획서 v2의 "전체 베스트 기록" = 옵션 A(스테이지별 개별
    // 비교) 기준. 세 슬롯 중 해당 스테이지를 한 번도 클리어한 적이 없으면
    // bOutHasRecord=false, 반환값은 0.
    UFUNCTION(BlueprintPure, Category = "Save")
    float GetBestStageTimeAcrossAllSlots(FName StageId, bool& bOutHasRecord) const;

    // [NEW] LoadGame 진행상황 동기화.
    // 현재 활성 슬롯(LoadFromSlot으로 막 로드된 상태)의 SavedCurrentStageIndex/
    // StageRecords[N].bCleared를 기준으로 "지금 다시 들어가야 할 레벨"을 계산한다.
    //   - 현재 스테이지(SavedCurrentStageIndex, 없으면 1)가 아직 클리어 전이면
    //     그 Stage_N으로 바로 재진입(Stage_1은 CliffSelection 없이, Stage_2/3는
    //     SelectedSeed/SelectedDifficulty가 이미 LoadFromSlot에서 복원돼 있어 동일한
    //     암벽으로 재생성된다)
    //   - 클리어까지 끝났고 다음 스테이지가 남아있으면 CliffSelection으로
    //   - Stage_3까지 클리어 완료면 Stage_3 재플레이
    UFUNCTION(BlueprintPure, Category = "Save")
    FName GetResumeLevelName() const;

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

    // 마지막으로 클리어한 스테이지 ID 설정 (DownfallGameMode::CompleteStage에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Stage")
    void SetLastClearedStageId(FName StageId) { LastClearedStageId = StageId; }

    // 마지막으로 클리어한 스테이지 ID 반환 (StageResult 화면에서 사용)
    UFUNCTION(BlueprintPure, Category = "Stage")
    FName GetLastClearedStageId() const { return LastClearedStageId; }

    // ========== Level Transition Flags ==========

    // 다음 스테이지 로드 시 Loading UI 표시 여부
    // CliffSelection → Stage 전환 시 true로 설정, Stage BeginPlay에서 false로 리셋
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetShowLoadingUI(bool bShow) { bShowLoadingUI = bShow; }

    UFUNCTION(BlueprintPure, Category = "UI")
    bool ShouldShowLoadingUI() const { return bShowLoadingUI; }


    // ========== UI / Menu Audio ==========
    // BP_DownfallGameInstance의 Class Defaults에서 설정한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Sound")
    TObjectPtr<USoundBase> UIButtonClickSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float UIButtonClickSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Sound")
    TObjectPtr<USoundBase> MainMenuBGM = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float MainMenuBGMVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound")
    TObjectPtr<USoundBase> CliffSelectionBGM = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float CliffSelectionBGMVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound")
    TObjectPtr<USoundBase> CliffSelectionRerollSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float CliffSelectionRerollSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound")
    TObjectPtr<USoundBase> CliffSelectionConfirmSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float CliffSelectionConfirmSoundVolumeMultiplier = 1.0f;

    // 암벽 선택 화면에서 A키로 왼쪽 암벽을 선택할 때 재생된다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound")
    TObjectPtr<USoundBase> CliffSelectionMoveLeftSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float CliffSelectionMoveLeftSoundVolumeMultiplier = 1.0f;

    // 암벽 선택 화면에서 D키로 오른쪽 암벽을 선택할 때 재생된다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound")
    TObjectPtr<USoundBase> CliffSelectionMoveRightSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float CliffSelectionMoveRightSoundVolumeMultiplier = 1.0f;

    // 스테이지를 클리어한 뒤 ResultLevel에서 재생되는 대기 화면 BGM.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StageResult|Sound")
    TObjectPtr<USoundBase> StageResultBGM = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StageResult|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float StageResultBGMVolumeMultiplier = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "UI|Sound")
    void PlayMenuBGM(UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "UI|Sound")
    void StopMenuBGM(float FadeOutSeconds = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "UI|Sound")
    void PlayUISound(UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);


    // 화면/레벨 전환 뒤에도 끝까지 들려야 하는 UI 전용 효과음.
    // 다음 persistent UI 효과음이 재생되면 이전 효과음은 즉시 정지한다.
    UFUNCTION(BlueprintCallable, Category = "UI|Sound")
    void PlayPersistentUISound(USoundBase* Sound, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

    // MenuBGMComponent가 끝나면 같은 곡을 다시 처음부터 재생해 BGM을 강제 반복한다.
    UFUNCTION()
    void HandleMenuBGMFinished();


    // 모든 UI 버튼의 OnClicked에 연결된다.
    // 기존 클릭음이 남아 있으면 끊고, 새 클릭음은 레벨 전환 뒤에도 계속 재생한다.
    UFUNCTION()
    void PlayUIButtonClickSound();

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

    // [DEBUG-FIX] CliffSelection<->Stage Seed 동기화 버그 수정용.
    // CliffSelection이 시드를 탐색(MGSearchSeedForTargets)할 때 실제로 사용한 Difficulty를
    // 그대로 저장해, 다음 Stage 레벨에서 같은 Difficulty로 Regenerate()하도록 한다.
    // 레벨에 배치된 MountainGenWorldActor의 Settings.Difficulty 값(에디터에서 설정한 값)에
    // 의존하지 않고, "실제로 시드를 찾을 때 쓴 Difficulty"를 단일 진실 공급원으로 삼는다.
    // 이렇게 하면 향후 스테이지가 추가되거나 난이도 매핑 규칙이 바뀌어도
    // CliffSelectionGameMode와 DownfallGameMode 양쪽에서 인덱스→난이도 매핑 로직을
    // 중복/동기화할 필요가 없다.
    UFUNCTION(BlueprintCallable, Category = "CliffSelection")
    void SetSelectedDifficulty(EMountainGenDifficulty Difficulty) { SelectedDifficulty = Difficulty; }

    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    EMountainGenDifficulty GetSelectedDifficulty() const { return SelectedDifficulty; }

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

    // 마지막으로 클리어한 스테이지 ID (StageResult 화면 표시용)
    UPROPERTY(BlueprintReadOnly, Category = "Stage")
    FName LastClearedStageId = NAME_None;

    // 세이브 슬롯 이름 생성
    FString GetSlotName(int32 SlotIndex) const;

    float GetConfiguredUISoundVolume(const USoundBase* Sound) const;

    // 스테이지 기록 찾기 (내부 함수)
    FStageTimeRecord* FindStageRecord(FName StageId);

    // ========== Cliff Selection (Runtime Only) ==========

    // CliffSelection 레벨에서 생성된 Seed 3개 (스테이지 전환 간 임시 데이터, 세이브 안 됨)
    UPROPERTY(BlueprintReadOnly, Category = "CliffSelection")
    TArray<int32> GeneratedSeeds;

    // 플레이어가 선택(Enter)한 Seed — 다음 스테이지 레벨 로드 시 사용
    UPROPERTY(BlueprintReadOnly, Category = "CliffSelection")
    int32 SelectedSeed = 0;

    // CliffSelection이 시드 탐색에 실제로 사용한 Difficulty — Stage 레벨에서 동일하게 재사용
    UPROPERTY(BlueprintReadOnly, Category = "CliffSelection")
    EMountainGenDifficulty SelectedDifficulty = EMountainGenDifficulty::Normal;


    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MenuBGMComponent = nullptr;


    // Menu / CliffSelection / StageResult BGM의 강제 반복 여부.
    bool bMenuBGMShouldLoop = false;

    // Confirm처럼 OpenLevel 뒤에도 유지되어야 하는 UI 효과음 전용 채널.
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> PersistentUIActionAudioComponent = nullptr;


    // UI 클릭음은 GameInstance가 보관하므로 OpenLevel 이후에도 사라지지 않는다.
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> UIButtonClickAudioComponent = nullptr;
};