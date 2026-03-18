#include "Core/DownfallGameInstance.h"
#include "Core/DownfallSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UDownfallGameInstance::Init()
{
    Super::Init();

    // 게임 시작 시 자동 로드
    LoadGame();

    UE_LOG(LogTemp, Warning, TEXT("=== DownfallGameInstance Initialized ==="));
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
    UDownfallSaveGame* SaveGameObject = Cast<UDownfallSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UDownfallSaveGame::StaticClass())
    );

    if (!SaveGameObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SaveGameObject"));
        return;
    }

    // 데이터 복사
    SaveGameObject->StageRecords = StageRecords;

    // 저장
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SaveSlotName, 0);

    if (bSuccess)
    {
    UE_LOG(LogTemp, Warning, TEXT("DownfallGameInstance: Game Saved - %d stage records"), StageRecords.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game"));
    }
}

void UDownfallGameInstance::LoadGame()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        UE_LOG(LogTemp, Log, TEXT("No save file found - starting fresh"));
        return;
    }

    UDownfallSaveGame* SaveGameObject = Cast<UDownfallSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)
    );

    if (!SaveGameObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load save game"));
        return;
    }

    // 데이터 복사
    StageRecords = SaveGameObject->StageRecords;

    UE_LOG(LogTemp, Warning, TEXT("DownfallGameInstance: Game Loaded - %d stage records"), StageRecords.Num());
}
