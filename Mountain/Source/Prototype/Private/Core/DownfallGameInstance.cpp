#include "Core/DownfallGameInstance.h"
#include "Core/DownfallSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

void UDownfallGameInstance::Init()
{
    Super::Init();

    // 기본 슬롯 0 로드
    CurrentSaveSlotIndex = 0;
    LoadGame();

    UE_LOG(LogTemp, Warning, TEXT("=== DownfallGameInstance Initialized ==="));
}

FString UDownfallGameInstance::GetSlotName(int32 SlotIndex) const
{
    return FString::Printf(TEXT("DownfallSave_Slot%d"), SlotIndex);
}

void UDownfallGameInstance::SetCurrentSaveSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid slot index: %d (must be 0-2)"), SlotIndex);
        return;
    }

    CurrentSaveSlotIndex = SlotIndex;
    UE_LOG(LogTemp, Warning, TEXT("Current save slot set to: %d"), SlotIndex);
}

void UDownfallGameInstance::SaveToSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid slot index: %d"), SlotIndex);
        return;
    }

    UDownfallSaveGame* SaveGameObject = Cast<UDownfallSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UDownfallSaveGame::StaticClass())
    );

    if (!SaveGameObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SaveGameObject"));
        return;
    }

    SaveGameObject->StageRecords = StageRecords;

    // 클리어한 스테이지 개수 계산
    int32 ClearedCount = 0;
    float TotalTime = 0.0f;
    for (const FStageTimeRecord& Record : StageRecords)
    {
        if (Record.bCleared)
        {
            ClearedCount++;
            TotalTime += Record.BestTime;
        }
    }

    SaveGameObject->ClearedStagesCount = ClearedCount;
    SaveGameObject->TotalPlayTime = TotalTime;
    SaveGameObject->LastSaveTime = FDateTime::Now();

    // [NEW] LoadGame 진행상황 동기화용 — 저장 시점의 진행 포인터를 같이 기록한다.
    SaveGameObject->SavedCurrentStageIndex = CurrentStageIndex;
    SaveGameObject->SavedSelectedSeed = SelectedSeed;
    SaveGameObject->SavedSelectedDifficulty = SelectedDifficulty;

    FString SlotName = GetSlotName(SlotIndex);

    // 저장 경로 출력
    FString SaveGamePath = FPaths::ProjectSavedDir() / TEXT("SaveGames") / (SlotName + TEXT(".sav"));
    UE_LOG(LogTemp, Warning, TEXT("===== SAVE FILE PATH ====="));
    UE_LOG(LogTemp, Warning, TEXT("Slot Name: %s"), *SlotName);
    UE_LOG(LogTemp, Warning, TEXT("Full Path: %s"), *SaveGamePath);
    UE_LOG(LogTemp, Warning, TEXT("=========================="));

    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game saved to slot %d - %d records"), SlotIndex, StageRecords.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save to slot %d"), SlotIndex);
    }
}

bool UDownfallGameInstance::LoadFromSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid slot index: %d"), SlotIndex);
        return false;
    }

    FString SlotName = GetSlotName(SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Log, TEXT("Slot %d has no save data"), SlotIndex);
        return false;
    }

    UDownfallSaveGame* SaveGameObject = Cast<UDownfallSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!SaveGameObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load from slot %d"), SlotIndex);
        return false;
    }

    StageRecords = SaveGameObject->StageRecords;
    CurrentSaveSlotIndex = SlotIndex;

    // [NEW] LoadGame 진행상황 동기화용 — 저장된 진행 포인터를 그대로 복원한다.
    // 이러면 GetResumeLevelName()이 올바른 레벨을 계산할 수 있고, Stage_2/3로 직접
    // 재진입하는 경우에도 SelectedSeed/SelectedDifficulty가 그 레벨의 GameMode가
    // 읽는 값과 동일하게 맞춰진다(CliffSelection을 다시 거치지 않아도 동일한 암벽 재생성).
    CurrentStageIndex = SaveGameObject->SavedCurrentStageIndex;
    SelectedSeed = SaveGameObject->SavedSelectedSeed;
    SelectedDifficulty = SaveGameObject->SavedSelectedDifficulty;

    UE_LOG(LogTemp, Warning, TEXT("Loaded from slot %d - %d records"), SlotIndex, StageRecords.Num());
    return true;
}

void UDownfallGameInstance::DeleteSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid slot index: %d"), SlotIndex);
        return;
    }

    FString SlotName = GetSlotName(SlotIndex);

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);
        if (bSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("Deleted save slot %d"), SlotIndex);
        }
    }
}

bool UDownfallGameInstance::DoesSaveSlotExist(int32 SlotIndex) const
{
    if (SlotIndex < 0 || SlotIndex > 2)
    {
        return false;
    }

    FString SlotName = GetSlotName(SlotIndex);
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

void UDownfallGameInstance::GetSlotInfo(int32 SlotIndex, float& OutTotalPlayTime, int32& OutClearedStages, FDateTime& OutLastSaveTime, bool& OutExists) const
{
    OutExists = false;
    OutTotalPlayTime = 0.0f;
    OutClearedStages = 0;
    OutLastSaveTime = FDateTime::MinValue();

    if (SlotIndex < 0 || SlotIndex > 2)
    {
        return;
    }

    FString SlotName = GetSlotName(SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return;
    }

    UDownfallSaveGame* SaveGameObject = Cast<UDownfallSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (SaveGameObject)
    {
        OutExists = true;
        OutTotalPlayTime = SaveGameObject->TotalPlayTime;
        OutClearedStages = SaveGameObject->ClearedStagesCount;
        OutLastSaveTime = SaveGameObject->LastSaveTime;
    }
}

TArray<FStageTimeRecord> UDownfallGameInstance::GetStageRecordsFromSlot(int32 SlotIndex) const
{
    if (SlotIndex < 0 || SlotIndex > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("GetStageRecordsFromSlot: Invalid slot index: %d"), SlotIndex);
        return TArray<FStageTimeRecord>();
    }

    // 현재 활성 슬롯과 같은 인덱스라면 디스크를 다시 읽을 필요 없이 메모리상의 최신
    // 상태(StageRecords)를 그대로 반환한다 — RecordStageTime() 직후 SaveGame()까지는
    // 끝났지만 아직 같은 틱 안이라 디스크 I/O를 한 번 더 할 필요가 없는 경우를 위함.
    if (SlotIndex == CurrentSaveSlotIndex)
    {
        return StageRecords;
    }

    FString SlotName = GetSlotName(SlotIndex);
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return TArray<FStageTimeRecord>();
    }

    UDownfallSaveGame* SaveGameObject = Cast<UDownfallSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!SaveGameObject)
    {
        return TArray<FStageTimeRecord>();
    }

    return SaveGameObject->StageRecords;
}

float UDownfallGameInstance::GetBestStageTimeAcrossAllSlots(FName StageId, bool& bOutHasRecord) const
{
    bOutHasRecord = false;
    float BestTime = 0.0f;

    if (StageId.IsNone())
    {
        return 0.0f;
    }

    for (int32 SlotIndex = 0; SlotIndex < 3; ++SlotIndex)
    {
        const TArray<FStageTimeRecord> SlotRecords = GetStageRecordsFromSlot(SlotIndex);

        for (const FStageTimeRecord& Record : SlotRecords)
        {
            if (Record.StageId != StageId || !Record.bCleared)
            {
                continue;
            }

            if (!bOutHasRecord || Record.BestTime < BestTime)
            {
                BestTime = Record.BestTime;
                bOutHasRecord = true;
            }
        }
    }

    return BestTime;
}

FName UDownfallGameInstance::GetResumeLevelName() const
{
    static const FName StageIds[3] = { FName("Stage_1"), FName("Stage_2"), FName("Stage_3") };

    // CurrentStageIndex 0(아직 어떤 CliffSelection도 거친 적 없음, 즉 처음부터 시작)도
    // Stage_1로 취급한다 — Stage_1은 CliffSelection 없이 바로 진입하는 스테이지다.
    const int32 Idx = FMath::Clamp(CurrentStageIndex, 1, 3);

    const bool bCurrentStageCleared = GetStageRecord(StageIds[Idx - 1]).bCleared;

    if (!bCurrentStageCleared)
    {
        // 이 스테이지를 아직 클리어 못 함 → 바로 재진입.
        // Stage_2/3라면 SavedSelectedSeed/SavedSelectedDifficulty가 LoadFromSlot에서
        // 이미 SelectedSeed/SelectedDifficulty로 복원돼 있어 동일한 암벽이 재생성된다.
        return StageIds[Idx - 1];
    }

    if (Idx >= 3)
    {
        // Stage_3까지 전부 클리어 완료 — 더 이상 진행할 스테이지가 없으므로 재플레이.
        return FName("Stage_3");
    }

    // 현재 스테이지는 클리어했고 다음 스테이지의 암벽은 아직 선택 전 — CliffSelection으로.
    return FName("CliffSelection");
}

void UDownfallGameInstance::RecordStageTime(FName StageId, float Time)
{
    if (StageId.IsNone() || Time <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid stage record: %s, Time: %.2f"),
            *StageId.ToString(), Time);
        return;
    }

    // 기존 기록 찾기
    FStageTimeRecord* ExistingRecord = FindStageRecord(StageId);

    if (ExistingRecord)
    {
        // 기존 기록 업데이트
        ExistingRecord->LastTime = Time;
        ExistingRecord->ClearCount++;
        ExistingRecord->bCleared = true;

        // 최고 기록 갱신
        if (ExistingRecord->BestTime == 0.0f || Time < ExistingRecord->BestTime)
        {
            ExistingRecord->BestTime = Time;
            UE_LOG(LogTemp, Warning, TEXT("DownfallGameInstance: Stage %s cleared! Time: %.2f seconds"),
                *StageId.ToString(), Time);
        }
    }
    else
    {
        // 새 기록 생성
        FStageTimeRecord NewRecord(StageId);
        NewRecord.LastTime = Time;
        NewRecord.BestTime = Time;
        NewRecord.ClearCount = 1;
        NewRecord.bCleared = true;

        StageRecords.Add(NewRecord);

        UE_LOG(LogTemp, Warning, TEXT("DownfallGameInstance: First clear of %s! Time: %.2f seconds"),
            *StageId.ToString(), Time);
    }

    // 자동 저장
    SaveGame();
}

FStageTimeRecord UDownfallGameInstance::GetStageRecord(FName StageId) const
{
    for (const FStageTimeRecord& Record : StageRecords)
    {
        if (Record.StageId == StageId)
        {
            return Record;
        }
    }

    // 기록 없으면 빈 Record 반환
    return FStageTimeRecord(StageId);
}

FStageTimeRecord* UDownfallGameInstance::FindStageRecord(FName StageId)
{
    for (FStageTimeRecord& Record : StageRecords)
    {
        if (Record.StageId == StageId)
        {
            return &Record;
        }
    }
    return nullptr;
}

void UDownfallGameInstance::GenerateNewSeeds()
{
    GeneratedSeeds.Reset();
    GeneratedSeeds.Reserve(3);

    FRandomStream Stream;
    Stream.GenerateNewSeed();

    for (int32 i = 0; i < 3; ++i)
    {
        // MountainGenWorldActor::SetSeed/RandomizeSeed와 동일하게 1 이상의 양수로 클램프
        int32 NewSeed = Stream.RandRange(1, TNumericLimits<int32>::Max());
        GeneratedSeeds.Add(NewSeed);
    }

    UE_LOG(LogTemp, Log, TEXT("DownfallGameInstance: Generated 3 new cliff seeds: %d, %d, %d"),
        GeneratedSeeds[0], GeneratedSeeds[1], GeneratedSeeds[2]);
}


void UDownfallGameInstance::SaveGame()
{
    SaveToSlot(CurrentSaveSlotIndex);
}

void UDownfallGameInstance::LoadGame()
{
    LoadFromSlot(CurrentSaveSlotIndex);
}



float UDownfallGameInstance::GetConfiguredUISoundVolume(const USoundBase* Sound) const
{
    if (!Sound)
    {
        return 0.0f;
    }

    if (Sound == UIButtonClickSound)
    {
        return FMath::Max(0.0f, UIButtonClickSoundVolumeMultiplier);
    }

    if (Sound == MainMenuBGM)
    {
        return FMath::Max(0.0f, MainMenuBGMVolumeMultiplier);
    }
    if (Sound == CliffSelectionBGM)
    {
        return FMath::Max(0.0f, CliffSelectionBGMVolumeMultiplier);
    }

    if (Sound == CliffSelectionRerollSound)
    {
        return FMath::Max(0.0f, CliffSelectionRerollSoundVolumeMultiplier);
    }

    if (Sound == CliffSelectionConfirmSound)
    {
        return FMath::Max(0.0f, CliffSelectionConfirmSoundVolumeMultiplier);
    }

    if (Sound == CliffSelectionMoveLeftSound)
    {
        return FMath::Max(0.0f, CliffSelectionMoveLeftSoundVolumeMultiplier);
    }

    if (Sound == CliffSelectionMoveRightSound)
    {
        return FMath::Max(0.0f, CliffSelectionMoveRightSoundVolumeMultiplier);
    }

    if (Sound == StageResultBGM)
    {
        return FMath::Max(0.0f, StageResultBGMVolumeMultiplier);
    }

    return 1.0f;
}

void UDownfallGameInstance::PlayMenuBGM(UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier)
{
    if (!WorldContextObject || !Sound)
    {
        return;
    }

    const float SafeVolume = FMath::Max(
        0.0f,
        VolumeMultiplier * GetConfiguredUISoundVolume(Sound)
    );

    // 같은 BGM이면 재생 위치는 유지하고 음량만 갱신한다.
    if (MenuBGMComponent && IsValid(MenuBGMComponent) && MenuBGMComponent->Sound == Sound)
    {
        bMenuBGMShouldLoop = true;
        MenuBGMComponent->SetVolumeMultiplier(SafeVolume);
        return;
    }

    StopMenuBGM(0.0f);

    MenuBGMComponent = UGameplayStatics::SpawnSound2D(
        WorldContextObject,
        Sound,
        SafeVolume,
        1.0f,
        0.0f,
        nullptr,
        true,
        false
    );

    if (MenuBGMComponent && IsValid(MenuBGMComponent))
    {
        bMenuBGMShouldLoop = true;
        MenuBGMComponent->bAutoDestroy = false;
        MenuBGMComponent->OnAudioFinished.AddUniqueDynamic(
            this,
            &UDownfallGameInstance::HandleMenuBGMFinished
        );
    }
}


void UDownfallGameInstance::StopMenuBGM(float FadeOutSeconds)
{
    bMenuBGMShouldLoop = false;

    if (!MenuBGMComponent || !IsValid(MenuBGMComponent))
    {
        MenuBGMComponent = nullptr;
        return;
    }

    MenuBGMComponent->Stop();
    MenuBGMComponent->DestroyComponent();
    MenuBGMComponent = nullptr;
}


void UDownfallGameInstance::PlayUISound(UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier)
{
    if (WorldContextObject && Sound)
    {
        const float SafeVolume = FMath::Max(
            0.0f,
            VolumeMultiplier * GetConfiguredUISoundVolume(Sound)
        );

        UGameplayStatics::PlaySound2D(
            WorldContextObject,
            Sound,
            SafeVolume,
            FMath::Max(0.01f, PitchMultiplier)
        );
    }
}


void UDownfallGameInstance::PlayUIButtonClickSound()
{
    if (!UIButtonClickSound)
    {
        return;
    }

    if (UIButtonClickAudioComponent && IsValid(UIButtonClickAudioComponent))
    {
        UIButtonClickAudioComponent->Stop();
        UIButtonClickAudioComponent->DestroyComponent();
        UIButtonClickAudioComponent = nullptr;
    }

    UWorld* CurrentWorld = GetWorld();
    if (!CurrentWorld)
    {
        return;
    }

    UIButtonClickAudioComponent = UGameplayStatics::SpawnSound2D(
        CurrentWorld,
        UIButtonClickSound,
        FMath::Max(0.0f, UIButtonClickSoundVolumeMultiplier),
        1.0f,
        0.0f,
        nullptr,
        true,
        false
    );
}


void UDownfallGameInstance::PlayUIButtonHoverSound()
{
    if (!UIButtonHoverSound)
    {
        return;
    }

    // 빠르게 다른 버튼으로 이동해도 Hover음이 누적되어 커지지 않게 한다.
    if (UIButtonHoverAudioComponent && IsValid(UIButtonHoverAudioComponent))
    {
        UIButtonHoverAudioComponent->Stop();
        UIButtonHoverAudioComponent->DestroyComponent();
        UIButtonHoverAudioComponent = nullptr;
    }

    UWorld* CurrentWorld = GetWorld();
    if (!CurrentWorld)
    {
        return;
    }

    UIButtonHoverAudioComponent = UGameplayStatics::SpawnSound2D(
        CurrentWorld,
        UIButtonHoverSound,
        FMath::Max(0.0f, UIButtonHoverSoundVolumeMultiplier),
        1.0f,
        0.0f,
        nullptr,
        true,
        false
    );
}


void UDownfallGameInstance::PlayPersistentUISound(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier)
{
    if (!Sound)
    {
        return;
    }

    if (PersistentUIActionAudioComponent && IsValid(PersistentUIActionAudioComponent))
    {
        PersistentUIActionAudioComponent->Stop();
        PersistentUIActionAudioComponent->DestroyComponent();
        PersistentUIActionAudioComponent = nullptr;
    }

    UWorld* CurrentWorld = GetWorld();
    if (!CurrentWorld)
    {
        return;
    }

    const float SafeVolume = FMath::Max(
        0.0f,
        VolumeMultiplier * GetConfiguredUISoundVolume(Sound)
    );

    PersistentUIActionAudioComponent = UGameplayStatics::SpawnSound2D(
        CurrentWorld,
        Sound,
        SafeVolume,
        FMath::Max(0.01f, PitchMultiplier),
        0.0f,
        nullptr,
        true,
        false
    );
}

void UDownfallGameInstance::HandleMenuBGMFinished()
{
    if (bMenuBGMShouldLoop &&
        MenuBGMComponent &&
        IsValid(MenuBGMComponent) &&
        MenuBGMComponent->Sound)
    {
        MenuBGMComponent->Play(0.0f);
    }
}