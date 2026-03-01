#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryTypes.h"
#include "InventoryComponent.generated.h"

class UItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPE_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 Capacity = 24;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FItemStack> Slots;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FItemStack>& GetSlots() const { return Slots; }

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetTotalCount(const UItemDefinition* Def) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(const UItemDefinition* Def, int32 Count, int32& OutAdded);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(const UItemDefinition* Def, int32 Count, int32& OutRemoved);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SwapSlots(int32 A, int32 B);

private:
    void EnsureSlotsSize();
    int32 AddToExistingStacks(const UItemDefinition* Def, int32 Count);
    int32 AddToEmptySlots(const UItemDefinition* Def, int32 Count);
};