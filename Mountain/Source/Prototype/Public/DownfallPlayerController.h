#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Item/InventoryWidget.h"
#include "DownfallPlayerController.generated.h"

class APawn;
class UInventoryComponent;
class UInventoryWidget;
class UItemDefinition;

USTRUCT(BlueprintType)
struct FStartupInventoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<const UItemDefinition> ItemDef = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 Count = 1;
};

UCLASS()
class PROTOTYPE_API ADownfallPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;

    UPROPERTY()
    TObjectPtr<UInventoryWidget> InventoryWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Startup")
    TArray<FStartupInventoryEntry> StartupItems;

private:
    UInventoryComponent* GetInventoryFromPawn() const;
    void DeferredInitAfterPossess();
    void CreateAndBindInventoryUI();
    void GiveStartupItems();
};