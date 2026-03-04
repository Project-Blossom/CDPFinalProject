#include "Item/InventoryComponent.h"
#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Slots.SetNum(SlotCount);
    OnInventoryChanged.Broadcast();
}

int32 UInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < Slots.Num(); ++i)
        if (Slots[i].IsEmpty())
            return i;
    return INDEX_NONE;
}

int32 UInventoryComponent::FindPartialStack(FName ItemId, int32 MaxStack) const
{
    if (ItemId == NAME_None || MaxStack <= 1) return INDEX_NONE;

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const FItemStack& S = Slots[i];
        if (S.IsValid() && S.ItemId == ItemId && !S.bHasInstance && S.Count < MaxStack)
            return i;
    }
    return INDEX_NONE;
}

bool UInventoryComponent::TryAdd(FName ItemId, int32 Count, bool bForceInstance)
{
    if (ItemId == NAME_None || Count <= 0) return false;

    UItemSubsystem* IS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
    const UItemDefinition* Def = IS ? IS->GetItemDefinitionById(ItemId) : nullptr;

    const int32 MaxStack = Def ? FMath::Max(1, Def->MaxStack) : 1;

    const bool bShouldInstance =
        bForceInstance ||
        (Def && Def->Type == EItemType::Weapon) ||
        (MaxStack == 1);

    int32 Remaining = Count;

    // 1) 스택형이면 기존 스택 채우기
    if (!bShouldInstance && MaxStack > 1)
    {
        while (Remaining > 0)
        {
            const int32 Idx = FindPartialStack(ItemId, MaxStack);
            if (Idx == INDEX_NONE) break;

            const int32 CanAdd = MaxStack - Slots[Idx].Count;
            const int32 AddNow = FMath::Min(CanAdd, Remaining);
            Slots[Idx].Count += AddNow;
            Remaining -= AddNow;
        }
    }

    // 2) 빈 슬롯에 새로 생성
    while (Remaining > 0)
    {
        const int32 Empty = FindEmptySlot();
        if (Empty == INDEX_NONE) break;

        FItemStack& S = Slots[Empty];
        S.Reset();
        S.ItemId = ItemId;

        if (bShouldInstance)
        {
            S.Count = 1;
            S.bHasInstance = true;
            S.Instance.InstanceId = FGuid::NewGuid();
            Remaining -= 1;
        }
        else
        {
            const int32 AddNow = FMath::Min(MaxStack, Remaining);
            S.Count = AddNow;
            Remaining -= AddNow;
        }
    }

    OnInventoryChanged.Broadcast();
    return (Remaining == 0);
}

bool UInventoryComponent::TryAddByDefinition(const UItemDefinition* Def, int32 Count, bool bForceInstance)
{
    if (!Def || Count <= 0) return false;
    return TryAdd(Def->ItemId, Count, bForceInstance);
}

bool UInventoryComponent::TryRemove(FName ItemId, int32 Count)
{
    if (ItemId == NAME_None || Count <= 0) return false;

    int32 Remaining = Count;

    for (int32 i = Slots.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        FItemStack& S = Slots[i];
        if (!S.IsValid() || S.ItemId != ItemId) continue;

        const int32 Take = FMath::Min(S.Count, Remaining);
        S.Count -= Take;
        Remaining -= Take;

        if (S.Count <= 0) S.Reset();
    }

    OnInventoryChanged.Broadcast();
    return (Remaining == 0);
}

bool UInventoryComponent::UseItem(int32 Index, AActor* User)
{
    if (!Slots.IsValidIndex(Index) || !User) return false;

    FItemStack& S = Slots[Index];
    if (!S.IsValid()) return false;

    UItemSubsystem* IS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
    UItemDefinition* Def = IS ? IS->GetItemDefinitionById(S.ItemId) : nullptr;
    if (!Def) return false;

    switch (Def->UseType)
    {
    case EItemUseType::Consume:
        BP_OnConsume(User, Def, 1);
        S.Count -= 1;
        if (S.Count <= 0) S.Reset();
        OnInventoryChanged.Broadcast();
        return true;

    case EItemUseType::PlaceActor:
        BP_OnPlace(User, Def);
        S.Count -= 1;
        if (S.Count <= 0) S.Reset();
        OnInventoryChanged.Broadcast();
        return true;

    case EItemUseType::Equip:
        if (!S.bHasInstance)
        {
            S.bHasInstance = true;
            S.Instance.InstanceId = FGuid::NewGuid();
            S.Instance.UpgradeLevel = 0;
            S.Count = 1;
        }
        BP_OnEquip(User, Def, S.Instance);
        return true;

    default:
        return false;
    }
}

bool UInventoryComponent::TransferTo(UInventoryComponent* Target, int32 FromIndex, int32 Count)
{
    if (!Target) return false;
    if (!Slots.IsValidIndex(FromIndex)) return false;
    if (Count <= 0) return false;

    FItemStack& From = Slots[FromIndex];
    if (!From.IsValid()) return false;

    const int32 MoveCount = FMath::Min(From.Count, Count);

    const bool bInstance = From.bHasInstance;
    if (bInstance && MoveCount != 1) return false;

    const bool bAdded = Target->TryAdd(From.ItemId, MoveCount, bInstance);
    if (!bAdded) return false;

    From.Count -= MoveCount;
    if (From.Count <= 0) From.Reset();

    OnInventoryChanged.Broadcast();
    return true;
}