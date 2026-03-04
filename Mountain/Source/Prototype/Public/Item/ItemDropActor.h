#pragma once

#include "CoreMinimal.h"
#include "Item/InventoryContainerActor.h"
#include "ItemDropActor.generated.h"

class USphereComponent;
class UItemDefinition;

UCLASS()
class PROTOTYPE_API AItemDropActor : public AInventoryContainerActor
{
    GENERATED_BODY()

public:
    AItemDropActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
    TObjectPtr<UItemDefinition> ItemDef;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
    int32 Count = 1;

protected:
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};