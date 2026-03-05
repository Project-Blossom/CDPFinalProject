#include "Item/InventoryWidget.h"

#include "Components/TextBlock.h"
#include "Item/InventoryComponent.h"
#include "Item/InventoryTypes.h"
#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UInventoryWidget::BindInventory(UInventoryComponent* InInventory)
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
    }

    Inventory = InInventory;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
    }

    Refresh();
}

void UInventoryWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
        Inventory = nullptr;
    }

    Super::NativeDestruct();
}

void UInventoryWidget::HandleInventoryChanged()
{
    Refresh();
}

void UInventoryWidget::Refresh()
{
    if (!SlotsText) return;

    if (!Inventory)
    {
        SlotsText->SetText(FText::FromString(TEXT("Inventory: (not bound)")));
        return;
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();
    UItemSubsystem* IS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;

    FString Out;
    Out += FString::Printf(TEXT("Inventory (%d slots)\n"), Slots.Num());
    Out += TEXT("----------------------\n");

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const FItemStack& S = Slots[i];

        if (S.IsEmpty())
        {
            Out += FString::Printf(TEXT("[%02d] (empty)\n"), i);
            continue;
        }

        UItemDefinition* Def = IS ? IS->GetItemDefinitionById(S.ItemId) : nullptr;
        const FString Name = Def ? Def->DisplayName.ToString() : S.ItemId.ToString();

        Out += FString::Printf(TEXT("[%02d] %s x%d\n"), i, *Name, S.Count);
    }

    SlotsText->SetText(FText::FromString(Out));
}