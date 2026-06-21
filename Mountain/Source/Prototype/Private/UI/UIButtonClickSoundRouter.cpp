#include "UI/UIButtonClickSoundRouter.h"
#include "Core/DownfallGameInstance.h"

void UUIButtonClickSoundRouter::Initialize(UDownfallGameInstance* InGameInstance)
{
    GameInstance = InGameInstance;
}

void UUIButtonClickSoundRouter::HandleButtonClicked()
{
    if (GameInstance && IsValid(GameInstance))
    {
        GameInstance->PlayUIButtonClickSound();
    }
}