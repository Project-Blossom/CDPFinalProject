#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UTextBlock;
class UInventoryComponent;

UCLASS(BlueprintType, Blueprintable)
class PROTOTYPE_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void BindInventory(UInventoryComponent* InInventory);

protected:
    virtual void NativeDestruct() override;

    UFUNCTION()
    void Refresh();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SlotsText = nullptr;

private:
    UPROPERTY()
    TObjectPtr<UInventoryComponent> Inventory = nullptr;

    UFUNCTION()
    void HandleInventoryChanged();
};