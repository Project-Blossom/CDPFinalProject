#include "UI/MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (NewGameButton)
    {
        NewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleNewGameClicked);
    }

    if (LoadGameButton)
    {
        LoadGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleLoadGameClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSettingsClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitClicked);
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
}

void UMainMenuWidget::OnNewGameClicked()
{
    HandleNewGameClicked();
}

void UMainMenuWidget::OnLoadGameClicked()
{
    HandleLoadGameClicked();
}

void UMainMenuWidget::OnSettingsClicked()
{
    HandleSettingsClicked();
}

void UMainMenuWidget::OnQuitClicked()
{
    HandleQuitClicked();
}

void UMainMenuWidget::HandleNewGameClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("New Game clicked - Opening Save Slot Selection"));

    // TODO: SaveSlotSelection 레벨로 이동 (아직 없음)
    // UGameplayStatics::OpenLevel(this, SaveSlotSelectionLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("SaveSlotSelection level not implemented yet"));
}

void UMainMenuWidget::HandleLoadGameClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Load Game clicked - Opening Save Slot Selection"));

    // TODO: SaveSlotSelection 레벨로 이동 (아직 없음)
    // UGameplayStatics::OpenLevel(this, SaveSlotSelectionLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("SaveSlotSelection level not implemented yet"));
}

void UMainMenuWidget::HandleSettingsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Settings clicked"));

    // TODO: Settings 레벨/UI로 이동 (아직 없음)
    
    UE_LOG(LogTemp, Warning, TEXT("Settings not implemented yet"));
}

void UMainMenuWidget::HandleQuitClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Quit clicked - Exiting game"));

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
}
