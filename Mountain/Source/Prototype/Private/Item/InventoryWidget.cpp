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
int32 UInventoryWidget::GetBoundSlotCount() const
{
    return Inventory ? Inventory->GetSlotCount() : 0;
}

int32 UInventoryWidget::GetLeftHandSlotIndex() const
{
    return Inventory ? Inventory->GetLeftReservedHandSlotIndex() : INDEX_NONE;
}

int32 UInventoryWidget::GetRightHandSlotIndex() const
{
    return Inventory ? Inventory->GetRightReservedHandSlotIndex() : INDEX_NONE;
}