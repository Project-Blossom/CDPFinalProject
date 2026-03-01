#include "Item/InventoryWidget.h"
#include "Components/TextBlock.h"
#include "Item/InventoryComponent.h"
#include "Item/InventoryTypes.h"
#include "Item/ItemDefinition.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    Refresh();
}

void UInventoryWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UInventoryWidget::BindInventory(UInventoryComponent* InInventory)
{
    if (Inventory == InInventory)
    {
        Refresh();
        return;
    }

    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Inventory = InInventory;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::Refresh);
    }

    Refresh();
}

void UInventoryWidget::Refresh()
{
    if (!SlotsText)
    {
        return;
    }

    if (!Inventory)
    {
        SlotsText->SetText(FText::FromString(TEXT("Inventory: (not bound)")));
        return;
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();

    FString Out;
    Out += FString::Printf(TEXT("Inventory (%d slots)\n"), Slots.Num());
    Out += TEXT("----------------------\n");

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const FItemStack& S = Slots[i];

        if (!S.IsValid())
        {
            Out += FString::Printf(TEXT("[%02d] (empty)\n"), i);
            continue;
        }

        const FString Name = (S.Def ? S.Def->DisplayName.ToString() : TEXT("(null def)"));
        Out += FString::Printf(TEXT("[%02d] %s x%d\n"), i, *Name, S.Count);
    }

    SlotsText->SetText(FText::FromString(Out));
}