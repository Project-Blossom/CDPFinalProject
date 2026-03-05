#include "Item/ChestActor.h"
#include "Item/InventoryComponent.h"

AChestActor::AChestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
    Inventory->SlotCount = 24;
}