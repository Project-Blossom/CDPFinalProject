#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;

UCLASS(BlueprintType, Blueprintable)
class PROTOTYPE_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void BindInventory(UInventoryComponent* InInventory);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UInventoryComponent* GetBoundInventory() const { return Inventory; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetBoundSlotCount() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetLeftHandSlotIndex() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetRightHandSlotIndex() const;

protected:
    virtual void NativeDestruct() override;

    UFUNCTION()
    void HandleInventoryChanged();

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void BP_OnInventoryBound(UInventoryComponent* InInventory);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void BP_OnInventoryChanged();

private:
    UPROPERTY()
    TObjectPtr<UInventoryComponent> Inventory = nullptr;
};