#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChestActor.generated.h"

class UInventoryComponent;

UCLASS()
class PROTOTYPE_API AChestActor : public AActor
{
    GENERATED_BODY()

public:
    AChestActor();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest")
    TObjectPtr<UInventoryComponent> Inventory = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest", meta = (ClampMin = "1"))
    int32 ChestSlotCount = 24;
};