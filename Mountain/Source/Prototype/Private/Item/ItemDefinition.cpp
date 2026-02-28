#include "Item/ItemDefinition.h"

UItemDefinition::UItemDefinition()
{
    Type = EItemType::Material;
    bStackable = true;
    MaxStack = 99;
}