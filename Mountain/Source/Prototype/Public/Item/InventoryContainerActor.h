#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryContainerActor.generated.h"

class UInventoryComponent;

UCLASS()
class PROTOTYPE_API AInventoryContainerActor : public AActor
{
    GENERATED_BODY()

public:
    AInventoryContainerActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryComponent> Inventory;
};