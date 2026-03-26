#include "UI/FreeRunSetupWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

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

    // 랜덤 Seed 생성 (1 ~ INT32_MAX)
    if (SeedTextBox)
    {
        int32 RandomSeed = FMath::RandRange(1, INT32_MAX);
        SeedTextBox->SetText(FText::AsNumber(RandomSeed));
    }
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
    SeedTextBox->SetText(FText::AsNumber(RandomSeed));

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

    UE_LOG(LogTemp, Warning, TEXT("FreeRun Setup - Seed: %d (corrected), Difficulty: %d"), Seed, SelectedDifficulty);

    // FreeRun 레벨로 이동 (Seed와 Difficulty를 URL 파라미터로 전달)
    FString LevelURL = FString::Printf(TEXT("%s?Seed=%d&Difficulty=%d"), 
        *FreeRunLevel.ToString(), Seed, SelectedDifficulty);
    
    UGameplayStatics::OpenLevel(this, FName(*LevelURL));
}

void UFreeRunSetupWidget::HandleBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Back to Main Menu"));
    UGameplayStatics::OpenLevel(this, MainMenuLevel);
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
