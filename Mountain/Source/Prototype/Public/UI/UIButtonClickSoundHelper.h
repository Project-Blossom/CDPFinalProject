#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Core/DownfallGameInstance.h"

namespace PrototypeUI
{
    inline void ApplyProjectButtonClickSound(
        UUserWidget* RootWidget,
        UButton* ExcludedButton = nullptr)
    {
        if (!RootWidget || !RootWidget->WidgetTree)
        {
            return;
        }

        UDownfallGameInstance* GameInstance =
            Cast<UDownfallGameInstance>(RootWidget->GetGameInstance());

        if (!GameInstance)
        {
            return;
        }

        TArray<UWidget*> AllWidgets;
        RootWidget->WidgetTree->GetAllWidgets(AllWidgets);

        for (UWidget* Widget : AllWidgets)
        {
            UButton* Button = Cast<UButton>(Widget);
            if (!Button || Button == ExcludedButton)
            {
                continue;
            }

            if (GameInstance->UIButtonClickSound)
            {
                Button->OnClicked.AddUniqueDynamic(
                    GameInstance,
                    &UDownfallGameInstance::PlayUIButtonClickSound
                );
            }

            if (GameInstance->UIButtonHoverSound)
            {
                Button->OnHovered.AddUniqueDynamic(
                    GameInstance,
                    &UDownfallGameInstance::PlayUIButtonHoverSound
                );
            }
        }
    }
}