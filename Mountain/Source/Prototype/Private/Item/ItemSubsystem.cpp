// Item/ItemSubsystem.cpp
#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"

void UItemSubsystem::BuildCacheIfNeeded()
{
    if (Cache.Num() > 0) return;

    for (const TSoftObjectPtr<UItemDefinition>& SoftDef : ItemList)
    {
        if (SoftDef.IsNull()) continue;

        UItemDefinition* Def = SoftDef.LoadSynchronous();
        if (!Def) continue;

        if (Def->ItemId == NAME_None) continue;

        Cache.Add(Def->ItemId, Def);
    }
}

UItemDefinition* UItemSubsystem::GetItemDefinitionById(FName ItemId)
{
    if (ItemId == NAME_None) return nullptr;

    BuildCacheIfNeeded();

    if (TObjectPtr<UItemDefinition>* Found = Cache.Find(ItemId))
    {
        return Found->Get();
    }
    return nullptr;
}