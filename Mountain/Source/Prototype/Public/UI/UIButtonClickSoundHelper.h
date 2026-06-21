#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Sound/SlateSound.h"
#include "Core/DownfallGameInstance.h"

// NativeConstruct에서 호출하면 해당 위젯 트리 안의 모든 UButton에
// GameInstance 공통 Pressed Sound를 적용한다.
namespace PrototypeUI
{
    inline void ApplyProjectButtonClickSound(UUserWidget* RootWidget)
    {
        if (!RootWidget || !RootWidget->WidgetTree)
        {
            return;
        }

        const UDownfallGameInstance* GameInstance =
            Cast<UDownfallGameInstance>(RootWidget->GetGameInstance());

        if (!GameInstance || !GameInstance->UIButtonClickSound)
        {
            return;
        }

        FSlateSound PressedSound;
        PressedSound.SetResourceObject(GameInstance->UIButtonClickSound);

        TArray<UWidget*> AllWidgets;
        RootWidget->WidgetTree->GetAllWidgets(AllWidgets);

        for (UWidget* Widget : AllWidgets)
        {
            UButton* Button = Cast<UButton>(Widget);
            if (!Button)
            {
                continue;
            }

            FButtonStyle Style = Button->GetStyle();
            Style.SetPressedSound(PressedSound);
            Button->SetStyle(Style);
        }
    }
}
