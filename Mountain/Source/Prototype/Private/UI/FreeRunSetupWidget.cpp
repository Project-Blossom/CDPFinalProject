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

    // 랜덤 Seed 생성
    if (SeedTextBox)
    {
        int32 RandomSeed = FMath::Rand();
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

    // 새로운 랜덤 Seed 생성
    int32 RandomSeed = FMath::Rand();
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

    // GameInstance에 Seed와 Difficulty 저장
    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        // GameInstance에 Seed와 Difficulty 저장
        // TODO: GameInstance에 변수 추가 필요
        GI->GetWorld()->GetAuthGameMode();
        
        UE_LOG(LogTemp, Warning, TEXT("FreeRun Setup - Seed: %d, Difficulty: %d"), Seed, SelectedDifficulty);
    }

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
