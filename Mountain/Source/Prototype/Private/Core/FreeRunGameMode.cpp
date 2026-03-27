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

    // ViewMode를 Lit으로 복원 (FreeRunSetup에서 Wireframe이었으므로)
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->ConsoleCommand(TEXT("viewmode lit"));
        UE_LOG(LogTemp, Warning, TEXT("FreeRun - ViewMode set to Lit"));
    }

    // URL 파라미터 파싱
    ParseURLParameters();

    // MountainGenWorldActor 설정 적용
    ApplyMountainSettings(ParsedSeed, ParsedDifficulty);
}

void AFreeRunGameMode::ParseURLParameters()
{
    // URL 옵션 문자열 가져오기 (GameModeBase의 OptionsString 멤버 변수 사용)
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    UE_LOG(LogTemp, Warning, TEXT("FreeRun - Parsing URL Parameters"));
    UE_LOG(LogTemp, Warning, TEXT("  Full URL Options: '%s'"), *OptionsString);

    // 수동 파싱 (UGameplayStatics::ParseOption이 제대로 작동하지 않는 경우 대비)
    TArray<FString> Params;
    OptionsString.ParseIntoArray(Params, TEXT("&"), true);

    for (const FString& Param : Params)
    {
        FString Key, Value;
        if (Param.Split(TEXT("="), &Key, &Value))
        {
            // ? 제거
            Key = Key.Replace(TEXT("?"), TEXT(""));
            
            UE_LOG(LogTemp, Log, TEXT("  Parsing: '%s' = '%s'"), *Key, *Value);

            if (Key.Equals(TEXT("Seed"), ESearchCase::IgnoreCase))
            {
                ParsedSeed = FCString::Atoi(*Value);
                UE_LOG(LogTemp, Warning, TEXT("  ✓ Parsed Seed: %d"), ParsedSeed);
            }
            else if (Key.Equals(TEXT("Difficulty"), ESearchCase::IgnoreCase))
            {
                ParsedDifficulty = FCString::Atoi(*Value);
                UE_LOG(LogTemp, Warning, TEXT("  ✓ Parsed Difficulty: %d"), ParsedDifficulty);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}

void AFreeRunGameMode::ApplyMountainSettings(int32 Seed, int32 Difficulty)
{
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    UE_LOG(LogTemp, Warning, TEXT("FreeRun - Applying Mountain Settings"));
    UE_LOG(LogTemp, Warning, TEXT("  Input Seed: %d"), Seed);
    UE_LOG(LogTemp, Warning, TEXT("  Input Difficulty: %d"), Difficulty);

    // MountainGenWorldActor 찾기
    AMountainGenWorldActor* MountainActor = nullptr;
    
    for (TActorIterator<AMountainGenWorldActor> It(GetWorld()); It; ++It)
    {
        MountainActor = *It;
        break;
    }

    if (!MountainActor)
    {
        UE_LOG(LogTemp, Error, TEXT("  ✗ MountainGenWorldActor not found in FreeRun level!"));
        UE_LOG(LogTemp, Warning, TEXT("========================================"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("  ✓ MountainGenWorldActor found: %s"), *MountainActor->GetName());

    // Seed 적용
    MountainActor->Settings.Seed = Seed;
    UE_LOG(LogTemp, Warning, TEXT("  ✓ Applied Seed: %d"), Seed);

    // Difficulty 적용 (0=Easy, 1=Normal, 2=Hard)
    switch (Difficulty)
    {
        case 0: // Easy
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Easy;
            UE_LOG(LogTemp, Warning, TEXT("  ✓ Applied Difficulty: Easy (0)"));
            break;

        case 1: // Normal
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Normal;
            UE_LOG(LogTemp, Warning, TEXT("  ✓ Applied Difficulty: Normal (1)"));
            break;

        case 2: // Hard
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Hard;
            UE_LOG(LogTemp, Warning, TEXT("  ✓ Applied Difficulty: Hard (2)"));
            break;

        default:
            MountainActor->Settings.Difficulty = EMountainGenDifficulty::Normal;
            UE_LOG(LogTemp, Warning, TEXT("  ⚠ Invalid Difficulty (%d), defaulting to Normal"), Difficulty);
            break;
    }

    // 재생성 전 최종 확인
    UE_LOG(LogTemp, Warning, TEXT("  Final Settings before Regenerate:"));
    UE_LOG(LogTemp, Warning, TEXT("    - Settings.Seed: %d"), MountainActor->Settings.Seed);
    UE_LOG(LogTemp, Warning, TEXT("    - Settings.Difficulty: %d"), (int32)MountainActor->Settings.Difficulty);

    // 재생성 (핵심!)
    MountainActor->Regenerate();
    
    UE_LOG(LogTemp, Warning, TEXT("  ✓ Regenerate() called!"));
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}
