#pragma once

#include "CoreMinimal.h"
#include "Item/InventoryContainerActor.h"
#include "ChestActor.generated.h"

UCLASS()
class PROTOTYPE_API AChestActor : public AInventoryContainerActor
{
    GENERATED_BODY()

public:
    AChestActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest", meta = (ClampMin = "1"))
    int32 ChestSlotCount = 24;
};