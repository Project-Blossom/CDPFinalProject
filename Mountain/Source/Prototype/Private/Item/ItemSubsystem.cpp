#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "Item/ItemRegistryDataAsset.h"

#include "UObject/SoftObjectPath.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ItemMap.Reset();

    static const FSoftObjectPath RegistryPath(TEXT("/Game/Item/DA_ItemRegistry.DA_ItemRegistry"));
    UItemRegistryDataAsset* Registry = Cast<UItemRegistryDataAsset>(RegistryPath.TryLoad());

    UE_LOG(LogTemp, Warning, TEXT("[ItemSubsystem] Registry=%s"), *GetNameSafe(Registry));

    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("[ItemSubsystem] Failed to load DA_ItemRegistry"));
        return;
    }

    for (const TSoftObjectPtr<UItemDefinition>& SoftDef : Registry->Items)
    {
        UItemDefinition* Def = SoftDef.LoadSynchronous();
        if (!Def)
        {
            UE_LOG(LogTemp, Error, TEXT("[ItemSubsystem] Failed to load item definition from registry"));
            continue;
        }

        if (Def->ItemId == NAME_None)
        {
            UE_LOG(LogTemp, Error, TEXT("[ItemSubsystem] Item definition has invalid ItemId: %s"), *GetNameSafe(Def));
            continue;
        }

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