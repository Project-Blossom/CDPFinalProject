#include "Core/DownfallGameInstance.h"
#include "Core/DownfallSaveGame.h"
#include "Kismet/GameplayStatics.h"

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

    FString SlotName = GetSlotName(SlotIndex);
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

void UDownfallGameInstance::SaveGame()
{
    SaveToSlot(CurrentSaveSlotIndex);
}

void UDownfallGameInstance::LoadGame()
{
    LoadFromSlot(CurrentSaveSlotIndex);
}
