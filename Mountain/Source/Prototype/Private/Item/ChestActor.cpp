#include "Item/ChestActor.h"
#include "Item/InventoryComponent.h"

AChestActor::AChestActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AChestActor::BeginPlay()
{
    Super::BeginPlay();

    if (Inventory)
    {
        Inventory->SlotCount = ChestSlotCount;
    }
}