#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPE_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 SlotCount = 24;

    UFUNCTION(BlueprintCallable)
    const TArray<FItemStack>& GetSlots() const { return Slots; }

    UFUNCTION(BlueprintCallable)
    bool AddItem(const UItemDefinition* Def, int32 Count);

    UFUNCTION(BlueprintCallable)
    bool RemoveItem(const UItemDefinition* Def, int32 Count);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<FItemStack> Slots;
};