#include "Core/FreeRunGameMode.h"
#include "MountainGenWorldActor.h"
#include "MountainGenSettings.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AFreeRunGameMode::AFreeRunGameMode()
{
    // 기본 Pawn 클래스 설정 (필요시 변경)
    // DefaultPawnClass = ADownfallCharacter::StaticClass();
}

void AFreeRunGameMode::BeginPlay()
{
    Super::BeginPlay();

    // URL 파라미터 파싱
    ParseURLParameters();

    // MountainGenWorldActor 설정 적용
    ApplyMountainSettings(ParsedSeed, ParsedDifficulty);
}

void AFreeRunGameMode::ParseURLParameters()
{
    // URL 옵션 문자열 가져오기 (GameModeBase의 OptionsString 멤버 변수 사용)
    UE_LOG(LogTemp, Warning, TEXT("FreeRun URL Options: %s"), *OptionsString);

    // Seed 파라미터 파싱
    FString SeedString = UGameplayStatics::ParseOption(OptionsString, TEXT("Seed"));
    if (!SeedString.IsEmpty())
    {
        ParsedSeed = FCString::Atoi(*SeedString);
        UE_LOG(LogTemp, Warning, TEXT("Parsed Seed: %d"), ParsedSeed);
    }

    // Difficulty 파라미터 파싱
    FString DifficultyString = UGameplayStatics::ParseOption(OptionsString, TEXT("Difficulty"));
    if (!DifficultyString.IsEmpty())
    {
        ParsedDifficulty = FCString::Atoi(*DifficultyString);
        UE_LOG(LogTemp, Warning, TEXT("Parsed Difficulty: %d"), ParsedDifficulty);
    }
}

void AFreeRunGameMode::ApplyMountainSettings(int32 Seed, int32 Difficulty)
{
    // MountainGenWorldActor 찾기
    AMountainGenWorldActor* MountainActor = nullptr;
    
    for (TActorIterator<AMountainGenWorldActor> It(GetWorld()); It; ++It)
    {
        MountainActor = *It;
        break;
    }

    if (!MountainActor)
    {
        UE_LOG(LogTemp, Error, TEXT("MountainGenWorldActor not found in FreeRun level!"));
        return;
    }

    // Seed 적용
    MountainActor->Settings.Seed = Seed;
    UE_LOG(LogTemp, Warning, TEXT("Applied Seed: %d"), Seed);

    // Difficulty 적용 (0=Easy, 1=Normal, 2=Hard)
    switch (Difficulty)
    {
        case 0: // Easy
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Easy;
            UE_LOG(LogTemp, Warning, TEXT("Applied Difficulty: Easy"));
            break;

        case 1: // Normal
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Normal;
            UE_LOG(LogTemp, Warning, TEXT("Applied Difficulty: Normal"));
            break;

        case 2: // Hard
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Hard;
            UE_LOG(LogTemp, Warning, TEXT("Applied Difficulty: Hard"));
            break;

        default:
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Normal;
            UE_LOG(LogTemp, Warning, TEXT("Invalid Difficulty, defaulting to Normal"));
            break;
    }

    // 지형 재생성 (MountainGenWorldActor에 Regenerate 함수가 있다면)
    // MountainActor->Regenerate();
    
    UE_LOG(LogTemp, Warning, TEXT("Mountain settings applied successfully!"));
}
