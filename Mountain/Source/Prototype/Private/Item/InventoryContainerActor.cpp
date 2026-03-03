#include "Item/InventoryContainerActor.h"
#include "Item/InventoryComponent.h"

AInventoryContainerActor::AInventoryContainerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}