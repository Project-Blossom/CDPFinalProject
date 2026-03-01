#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

class UTextBlock;
class UInventoryComponent;

#include "InventoryWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROTOTYPE_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void BindInventory(UInventoryComponent* InInventory);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void Refresh();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SlotsText = nullptr;

private:
    UPROPERTY()
    TObjectPtr<UInventoryComponent> Inventory = nullptr;
};