#include "Item/ItemDefinition.h"

UItemDefinition::UItemDefinition()
{
    ItemId = NAME_None;
    DisplayName = FText::FromString(TEXT("Unknown Item"));
    Type = EItemType::Etc;
    MaxStack = 1;
}