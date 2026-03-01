#include "Item/InventoryComponent.h"
#include "Item/ItemDefinition.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::EnsureSlotsSize()
{
    if (Capacity < 1) Capacity = 1;
    if (Slots.Num() != Capacity)
    {
        Slots.SetNum(Capacity);
    }
}

int32 UInventoryComponent::GetTotalCount(const UItemDefinition* Def) const
{
    if (!Def) return 0;

    int32 Sum = 0;
    for (const FItemStack& S : Slots)
    {
        if (S.Def == Def && S.Count > 0)
        {
            Sum += S.Count;
        }
    }
    return Sum;
}

bool UInventoryComponent::AddItem(const UItemDefinition* Def, int32 Count, int32& OutAdded)
{
    OutAdded = 0;
    if (!Def || Count <= 0) return false;

    EnsureSlotsSize();

    int32 Remaining = Count;

    Remaining -= AddToExistingStacks(Def, Remaining);

    Remaining -= AddToEmptySlots(Def, Remaining);

    OutAdded = Count - Remaining;
    if (OutAdded > 0)
    {
        OnInventoryChanged.Broadcast();
        return true;
    }
    return false;
}

int32 UInventoryComponent::AddToExistingStacks(const UItemDefinition* Def, int32 Count)
{
    if (!Def || Count <= 0) return 0;
    if (!Def->bStackable) return 0;

    int32 Added = 0;

    for (FItemStack& S : Slots)
    {
        if (Count <= 0) break;
        if (S.Def != Def || S.Count <= 0) continue;

        const int32 Space = FMath::Max(0, Def->MaxStack - S.Count);
        if (Space <= 0) continue;

        const int32 Delta = FMath::Min(Space, Count);
        S.Count += Delta;
        Count -= Delta;
        Added += Delta;
    }

    return Added;
}

int32 UInventoryComponent::AddToEmptySlots(const UItemDefinition* Def, int32 Count)
{
    if (!Def || Count <= 0) return 0;

    int32 Added = 0;

    for (FItemStack& S : Slots)
    {
        if (Count <= 0) break;
        if (S.IsValid()) continue;

        S.Def = Def;

        if (Def->bStackable)
        {
            const int32 Put = FMath::Min(Def->MaxStack, Count);
            S.Count = Put;
            Count -= Put;
            Added += Put;
        }
        else
        {
            S.Count = 1;
            Added += 1;
            Count -= 1;
        }
    }

    return Added;
}

bool UInventoryComponent::RemoveItem(const UItemDefinition* Def, int32 Count, int32& OutRemoved)
{
    OutRemoved = 0;
    if (!Def || Count <= 0) return false;

    EnsureSlotsSize();

    int32 Remaining = Count;

    for (FItemStack& S : Slots)
    {
        if (Remaining <= 0) break;
        if (S.Def != Def || S.Count <= 0) continue;

        const int32 Delta = FMath::Min(S.Count, Remaining);
        S.Count -= Delta;
        Remaining -= Delta;
        OutRemoved += Delta;

        if (S.Count <= 0)
        {
            S.Def = nullptr;
            S.Count = 0;
        }
    }

    if (OutRemoved > 0)
    {
        OnInventoryChanged.Broadcast();
        return true;
    }
    return false;
}

bool UInventoryComponent::SwapSlots(int32 A, int32 B)
{
    EnsureSlotsSize();
    if (!Slots.IsValidIndex(A) || !Slots.IsValidIndex(B) || A == B) return false;

    Swap(Slots[A], Slots[B]);
    OnInventoryChanged.Broadcast();
    return true;
}