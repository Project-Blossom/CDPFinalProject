#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Item/InventoryTypes.h"
#include "InventorySaveGame.generated.h"

UCLASS()
class PROTOTYPE_API UInventorySaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int32 SlotCount = 24;

    UPROPERTY()
    TArray<FItemStack> Slots;
};