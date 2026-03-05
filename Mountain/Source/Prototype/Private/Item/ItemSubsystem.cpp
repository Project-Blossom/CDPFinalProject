#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ItemMap.Reset();

    for (UItemDefinition* Def : ItemList)
    {
        if (!Def) continue;
        if (Def->ItemId == NAME_None) continue;

        ItemMap.Add(Def->ItemId, Def);
    }
}

UItemDefinition* UItemSubsystem::GetItemDefinitionById(FName ItemId) const
{
    if (ItemId == NAME_None) return nullptr;

    if (const TObjectPtr<UItemDefinition>* Found = ItemMap.Find(ItemId))
    {
        return Found->Get();
    }
    return nullptr;
}