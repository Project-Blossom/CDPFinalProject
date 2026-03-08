#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DownfallPlayerController.generated.h"

class APawn;
class UInventoryComponent;
class UInventoryWidget;
class UItemDefinition;

UCLASS()
class PROTOTYPE_API ADownfallPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;

    UPROPERTY()
    TObjectPtr<UInventoryWidget> InventoryWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Test")
    TObjectPtr<const UItemDefinition> TestAnchorItemDef = nullptr;

private:
    UInventoryComponent* GetInventoryFromPawn() const;
    void DeferredInitAfterPossess();
    void CreateAndBindInventoryUI();
    void GiveTestItemsIfNeeded();
};