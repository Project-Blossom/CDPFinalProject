#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ItemMap.Reset();
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

    // 임시: 앵커만 필요할 때 즉시 로드
    if (ItemId == FName(TEXT("Anchor")))
    {
        UItemDefinition* AnchorDef = LoadObject<UItemDefinition>(
            nullptr,
            TEXT("/Game/Item/DA_Item_Anchor.DA_Item_Anchor")
        );

        if (AnchorDef)
        {
            ItemMap.Add(ItemId, AnchorDef);
            return AnchorDef;
        }
    }

    return nullptr;
}