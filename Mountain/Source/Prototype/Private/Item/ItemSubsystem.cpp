#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

UItemDefinition* UItemSubsystem::GetItemDefinition(const FPrimaryAssetId& Id) const
{
    if (!Id.IsValid()) return nullptr;

    UAssetManager& AM = UAssetManager::Get();

    if (UObject* LoadedObj = AM.GetPrimaryAssetObject(Id))
    {
        return Cast<UItemDefinition>(LoadedObj);
    }

    TSharedPtr<FStreamableHandle> Handle = AM.LoadPrimaryAsset(Id);
    if (!Handle.IsValid())
    {
        return nullptr;
    }

    Handle->WaitUntilComplete();

    UObject* LoadedObj = AM.GetPrimaryAssetObject(Id);
    return Cast<UItemDefinition>(LoadedObj);
}