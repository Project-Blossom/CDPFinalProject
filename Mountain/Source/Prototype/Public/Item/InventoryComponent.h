#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

class UItemDefinition;

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

    // ---------- ÇÙ½É API ----------
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAdd(const FPrimaryAssetId& ItemId, int32 Count, bool bForceInstance = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryRemove(const FPrimaryAssetId& ItemId, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool MoveOrSwap(int32 FromIndex, int32 ToIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SplitStack(int32 FromIndex, int32 SplitCount);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TransferTo(UInventoryComponent* Other, int32 FromIndex, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(int32 Index, AActor* User);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    void BP_OnConsume(AActor* User, const UItemDefinition* Def, int32 Count);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    void BP_OnEquip(AActor* User, const UItemDefinition* Def, const FItemInstanceData& Instance);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    void BP_OnPlace(AActor* User, const UItemDefinition* Def);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<FItemStack> Slots;

    int32 FindEmptySlot() const;
    int32 FindPartialStack(const FPrimaryAssetId& ItemId, int32 MaxStack) const;
};