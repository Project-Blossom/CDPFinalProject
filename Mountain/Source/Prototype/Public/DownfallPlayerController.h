#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DownfallPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

class UInventoryComponent;
class UInventoryWidget;
class UItemDefinition;

struct FInputActionValue;

UCLASS()
class PROTOTYPE_API ADownfallPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn* InPawn) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> PlayerMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> UseItemAction;

    // UI
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;

    UPROPERTY()
    TObjectPtr<UInventoryWidget> InventoryWidget;

    // 테스트 지급 앵커
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Test")
    TObjectPtr<const UItemDefinition> TestAnchorItemDef;

private:
    UInventoryComponent* GetInventoryFromPawn() const;

    void DeferredInitAfterPossess();

    void CreateAndBindInventoryUI();
    void GiveTestItemsIfNeeded();

    void OnUseItemTriggered(const FInputActionValue& Value);
};