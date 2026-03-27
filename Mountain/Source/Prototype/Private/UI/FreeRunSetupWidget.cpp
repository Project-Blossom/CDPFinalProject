#include "UI/FreeRunSetupWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "MountainGenWorldActor.h"

void UFreeRunSetupWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (EasyButton)
    {
        EasyButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleEasyClicked);
    }

    if (NormalButton)
    {
        NormalButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleNormalClicked);
    }

    if (HardButton)
    {
        HardButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleHardClicked);
    }

    if (RandomSeedButton)
    {
        RandomSeedButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleRandomSeedClicked);
    }

    if (GenerateButton)
    {
        GenerateButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleGenerateClicked);
    }

    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleStartClicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UFreeRunSetupWidget::HandleBackClicked);
    }

    // Seed TextBox 커밋 이벤트 바인딩 (엔터 키 입력 시)
    if (SeedTextBox)
    {
        SeedTextBox->OnTextCommitted.AddDynamic(this, &UFreeRunSetupWidget::HandleSeedTextCommitted);
    }

    // Input Mode 설정
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    // 초기 Difficulty 표시
    UpdateDifficultyUI();

    // 랜덤 Seed 생성 (1 ~ INT32_MAX, 쉼표 없이)
    if (SeedTextBox)
    {
        int32 RandomSeed = FMath::RandRange(1, INT32_MAX);
        SeedTextBox->SetText(FText::FromString(FString::FromInt(RandomSeed)));
    }

    // 미리보기 Actor 찾기
    FindPreviewMountain();
}

void UFreeRunSetupWidget::OnEasyClicked()
{
    HandleEasyClicked();
}

void UFreeRunSetupWidget::OnNormalClicked()
{
    HandleNormalClicked();
}

void UFreeRunSetupWidget::OnHardClicked()
{
    HandleHardClicked();
}

void UFreeRunSetupWidget::OnRandomSeedClicked()
{
    HandleRandomSeedClicked();
}

void UFreeRunSetupWidget::OnGenerateClicked()
{
    HandleGenerateClicked();
}

void UFreeRunSetupWidget::OnStartClicked()
{
    HandleStartClicked();
}

void UFreeRunSetupWidget::OnBackClicked()
{
    HandleBackClicked();
}

void UFreeRunSetupWidget::HandleEasyClicked()
{
    SelectedDifficulty = 0;
    UpdateDifficultyUI();
    UE_LOG(LogTemp, Log, TEXT("Difficulty set to Easy"));
}

void UFreeRunSetupWidget::HandleNormalClicked()
{
    SelectedDifficulty = 1;
    UpdateDifficultyUI();
    UE_LOG(LogTemp, Log, TEXT("Difficulty set to Normal"));
}

void UFreeRunSetupWidget::HandleHardClicked()
{
    SelectedDifficulty = 2;
    UpdateDifficultyUI();
    UE_LOG(LogTemp, Log, TEXT("Difficulty set to Hard"));
}

void UFreeRunSetupWidget::HandleRandomSeedClicked()
{
    if (!SeedTextBox)
        return;

    // 새로운 랜덤 Seed 생성 (1 ~ INT32_MAX)
    int32 RandomSeed = FMath::RandRange(1, INT32_MAX);
    
    // 쉼표 없이 숫자만 표시
    SeedTextBox->SetText(FText::FromString(FString::FromInt(RandomSeed)));

    UE_LOG(LogTemp, Log, TEXT("Random Seed generated: %d"), RandomSeed);
}

void UFreeRunSetupWidget::HandleGenerateClicked()
{
    if (!SeedTextBox)
    {
        UE_LOG(LogTemp, Error, TEXT("SeedTextBox is null!"));
        return;
    }

    // Seed 가져오기
    FString SeedString = SeedTextBox->GetText().ToString();
    int32 Seed = FCString::Atoi(*SeedString);

    // Seed 보정 (MountainGen 규칙: 최소값 1)
    Seed = FMath::Max(1, Seed);

    UE_LOG(LogTemp, Warning, TEXT("Generate clicked - Seed: %d, Difficulty: %d"), Seed, SelectedDifficulty);

    // 미리보기 업데이트
    UpdatePreview(Seed, SelectedDifficulty);

    // 나중에 별도 Start 버튼으로 분리 가능
    // FString LevelURL = FString::Printf(TEXT("%s?Seed=%d&Difficulty=%d"), 
    //     *FreeRunLevel.ToString(), Seed, SelectedDifficulty);
    // UGameplayStatics::OpenLevel(this, FName(*LevelURL));
}

void UFreeRunSetupWidget::HandleBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Back to Main Menu"));
    UGameplayStatics::OpenLevel(this, MainMenuLevel);
}

void UFreeRunSetupWidget::HandleStartClicked()
{
    if (!SeedTextBox)
    {
        UE_LOG(LogTemp, Error, TEXT("SeedTextBox is null!"));
        return;
    }

    // SeedTextBox에서 Seed 가져오기
    // 이 값은 Generate 후 SeedSearch로 보정된 최종 Seed 값임
    FString SeedString = SeedTextBox->GetText().ToString();
    int32 Seed = FCString::Atoi(*SeedString);

    // Seed 보정 (MountainGen 규칙: 최소값 1)
    Seed = FMath::Max(1, Seed);

    UE_LOG(LogTemp, Warning, TEXT("Start Climbing clicked!"));
    UE_LOG(LogTemp, Warning, TEXT("  - Seed (from TextBox, post-SeedSearch): %d"), Seed);
    UE_LOG(LogTemp, Warning, TEXT("  - Difficulty: %d"), SelectedDifficulty);
    UE_LOG(LogTemp, Warning, TEXT("Moving to FreeRun level..."));

    // FreeRun 레벨로 이동 (Seed와 Difficulty를 URL 파라미터로 전달)
    FString LevelURL = FString::Printf(TEXT("%s?Seed=%d&Difficulty=%d"), 
        *FreeRunLevel.ToString(), Seed, SelectedDifficulty);
    
    UGameplayStatics::OpenLevel(this, FName(*LevelURL));
}

void UFreeRunSetupWidget::HandleSeedTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    // 엔터 키로 커밋된 경우에만 보정
    if (CommitMethod != ETextCommit::OnEnter && CommitMethod != ETextCommit::OnUserMovedFocus)
        return;

    if (!SeedTextBox)
        return;

    FString SeedString = Text.ToString();
    
    // 빈 문자열이면 기본값 1로 설정
    if (SeedString.IsEmpty())
    {
        SeedTextBox->SetText(FText::AsNumber(1));
        UE_LOG(LogTemp, Log, TEXT("Empty seed auto-corrected to: 1"));
        return;
    }

    // 숫자로 변환
    int32 Seed = FCString::Atoi(*SeedString);

    // Seed 보정 (MountainGen 규칙: 최소값 1)
    int32 CorrectedSeed = FMath::Max(1, Seed);

    // 입력값과 보정값이 다르면 자동 보정
    if (Seed != CorrectedSeed)
    {
        SeedTextBox->SetText(FText::AsNumber(CorrectedSeed));
        UE_LOG(LogTemp, Log, TEXT("Seed auto-corrected: %d -> %d"), Seed, CorrectedSeed);
    }
}

void UFreeRunSetupWidget::UpdateDifficultyUI()
{
    if (!SelectedDifficultyText)
        return;

    FString DifficultyName;
    switch (SelectedDifficulty)
    {
        case 0: DifficultyName = TEXT("Easy"); break;
        case 1: DifficultyName = TEXT("Normal"); break;
        case 2: DifficultyName = TEXT("Hard"); break;
        default: DifficultyName = TEXT("Unknown"); break;
    }

    SelectedDifficultyText->SetText(FText::FromString(FString::Printf(TEXT("Selected: %s"), *DifficultyName)));
}

void UFreeRunSetupWidget::FindPreviewMountain()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null in FindPreviewMountain"));
        return;
    }

    // MountainGenWorldActor 찾기
    for (TActorIterator<AMountainGenWorldActor> It(World); It; ++It)
    {
        PreviewMountain = *It;
        UE_LOG(LogTemp, Warning, TEXT("Preview Mountain found: %s"), *PreviewMountain->GetName());
        break;  // 첫 번째 것만 사용
    }

    if (!PreviewMountain)
    {
        UE_LOG(LogTemp, Error, TEXT("Preview Mountain not found in FreeRunSetup level!"));
    }
}

void UFreeRunSetupWidget::UpdatePreview(int32 Seed, int32 Difficulty)
{
    if (!PreviewMountain)
    {
        UE_LOG(LogTemp, Error, TEXT("PreviewMountain is null - cannot update preview"));
        return;
    }

    // Seed 적용
    PreviewMountain->Settings.Seed = Seed;

    // Difficulty 적용
    switch (Difficulty)
    {
        case 0:
            PreviewMountain->Settings.Difficulty = EMountainGenDifficulty::Easy;
            break;
        case 1:
            PreviewMountain->Settings.Difficulty = EMountainGenDifficulty::Normal;
            break;
        case 2:
            PreviewMountain->Settings.Difficulty = EMountainGenDifficulty::Hard;
            break;
        default:
            PreviewMountain->Settings.Difficulty = EMountainGenDifficulty::Normal;
            break;
    }

    // 재생성
    PreviewMountain->Regenerate();

    UE_LOG(LogTemp, Warning, TEXT("Preview Regenerate called - Input Seed: %d, Difficulty: %d"), Seed, Difficulty);

    // Regenerate()는 비동기이므로 완료 후 Seed를 읽어야 함
    // Timer로 2초 후 Seed 업데이트 (SeedSearch 완료 대기)
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            SeedUpdateTimerHandle,
            this,
            &UFreeRunSetupWidget::UpdateSeedFromPreview,
            2.0f,  // 2초 후 실행
            false  // 반복 안 함
        );
    }
}

void UFreeRunSetupWidget::UpdateSeedFromPreview()
{
    if (!PreviewMountain || !SeedTextBox)
    {
        UE_LOG(LogTemp, Error, TEXT("PreviewMountain or SeedTextBox is null"));
        return;
    }

    // 재생성 후 실제 적용된 Seed 값 가져오기 (SeedSearch로 변경될 수 있음)
    int32 FinalSeed = PreviewMountain->Settings.Seed;
    
    // TextBox 업데이트 (쉼표 없이)
    SeedTextBox->SetText(FText::FromString(FString::FromInt(FinalSeed)));
    
    UE_LOG(LogTemp, Warning, TEXT("Final Seed after SeedSearch: %d"), FinalSeed);
}
