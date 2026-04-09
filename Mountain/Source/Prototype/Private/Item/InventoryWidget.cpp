#include "Item/InventoryWidget.h"
#include "Item/InventoryComponent.h"

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

    BP_OnInventoryBound(Inventory);
    BP_OnInventoryChanged();
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
    BP_OnInventoryChanged();
}