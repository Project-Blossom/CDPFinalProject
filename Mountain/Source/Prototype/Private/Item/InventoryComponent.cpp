#include "Item/InventoryComponent.h"
#include "Item/ItemDefinition.h"

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    Slots.SetNum(SlotCount);
    OnInventoryChanged.Broadcast();
}

static int32 FindSlotWithDef(const TArray<FItemStack>& Slots, const UItemDefinition* Def)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
        if (Slots[i].Def == Def && Slots[i].Count > 0)
            return i;
    return INDEX_NONE;
}

static int32 FindEmptySlot(const TArray<FItemStack>& Slots)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
        if (!Slots[i].IsValid())
            return i;
    return INDEX_NONE;
}

bool UInventoryComponent::AddItem(const UItemDefinition* Def, int32 Count)
{
    if (!Def || Count <= 0) return false;

    int32 Remaining = Count;

    while (Remaining > 0)
    {
        const int32 Idx = FindSlotWithDef(Slots, Def);
        if (Idx == INDEX_NONE) break;

        const int32 MaxStack = FMath::Max(1, Def->MaxStack);
        const int32 CanAdd = FMath::Max(0, MaxStack - Slots[Idx].Count);
        if (CanAdd <= 0) break;

        const int32 AddNow = FMath::Min(CanAdd, Remaining);
        Slots[Idx].Count += AddNow;
        Remaining -= AddNow;
    }

    while (Remaining > 0)
    {
        const int32 Empty = FindEmptySlot(Slots);
        if (Empty == INDEX_NONE) break;

        const int32 MaxStack = FMath::Max(1, Def->MaxStack);
        const int32 AddNow = FMath::Min(MaxStack, Remaining);

        Slots[Empty].Def = Def;
        Slots[Empty].Count = AddNow;

        Remaining -= AddNow;
    }

    const bool bAllAdded = (Remaining == 0);
    OnInventoryChanged.Broadcast();
    return bAllAdded;
}

bool UInventoryComponent::RemoveItem(const UItemDefinition* Def, int32 Count)
{
    if (!Def || Count <= 0) return false;

    int32 Remaining = Count;

    for (int32 i = Slots.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        if (Slots[i].Def != Def || Slots[i].Count <= 0) continue;

        const int32 Take = FMath::Min(Slots[i].Count, Remaining);
        Slots[i].Count -= Take;
        Remaining -= Take;

        if (Slots[i].Count <= 0)
        {
            Slots[i].Def = nullptr;
            Slots[i].Count = 0;
        }
    }

    const bool bAllRemoved = (Remaining == 0);
    OnInventoryChanged.Broadcast();
    return bAllRemoved;
}