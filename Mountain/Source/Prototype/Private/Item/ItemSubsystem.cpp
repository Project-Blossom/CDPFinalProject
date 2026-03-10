#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "Item/ItemRegistryDataAsset.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ItemMap.Reset();

    UItemRegistryDataAsset* Registry = LoadObject<UItemRegistryDataAsset>(
        nullptr,
        TEXT("/Game/Item/DA_ItemRegistry.DA_ItemRegistry")
    );

    UE_LOG(LogTemp, Warning, TEXT("[ItemSubsystem] Registry=%s"), *GetNameSafe(Registry));

    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("[ItemSubsystem] Failed to load DA_ItemRegistry"));
        return;
    }

    for (const TSoftObjectPtr<UItemDefinition>& SoftDef : Registry->Items)
    {
        UItemDefinition* Def = SoftDef.LoadSynchronous();
        if (!Def) continue;
        if (Def->ItemId == NAME_None) continue;

        ItemMap.Add(Def->ItemId, Def);

        UE_LOG(LogTemp, Warning, TEXT("[ItemSubsystem] Registered ItemId=%s"), *Def->ItemId.ToString());
    }
}

UItemDefinition* UItemSubsystem::GetItemDefinitionById(FName ItemId) const
{
    if (ItemId == NAME_None)
    {
        return nullptr;
    }

    if (const TObjectPtr<UItemDefinition>* Found = ItemMap.Find(ItemId))
    {
        return Found->Get();
    }

    return nullptr;
}