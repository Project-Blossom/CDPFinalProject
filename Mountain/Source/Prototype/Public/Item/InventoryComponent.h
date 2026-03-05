#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/InventoryTypes.h"
#include "InventoryComponent.generated.h"

class UItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPE_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    virtual void BeginPlay() override;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 SlotCount = 24;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Place", meta = (ClampMin = "1.0"))
    float PlaceRangeCm = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Place", meta = (ClampMin = "1.0"))
    float PlaceTraceDistanceCm = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Place", meta = (ClampMin = "0.0"))
    float PlaceEmbedCm = 5.f;

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FItemStack>& GetSlots() const { return Slots; }

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAdd(FName ItemId, int32 Count, bool bForceInstance = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddByDefinition(const UItemDefinition* Def, int32 Count, bool bForceInstance = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryRemove(FName ItemId, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(int32 Index, AActor* User);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TransferTo(UInventoryComponent* Target, int32 FromIndex, int32 Count);

    // ---- Save/Load ----
    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    bool SaveToSlot(const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    bool LoadFromSlot(const FString& SlotName, int32 UserIndex = 0);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    void BP_OnConsume(AActor* User, UItemDefinition* Def, int32 Count);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    void BP_OnEquip(AActor* User, UItemDefinition* Def, const FItemInstanceData& Instance);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    bool BP_OnPlace(AActor* User, UItemDefinition* Def, const FTransform& SpawnTransform);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Use")
    void BP_OnUseFailed(AActor* User, const FText& Reason);

private:
    UPROPERTY()
    TArray<FItemStack> Slots;

private:
    int32 FindEmptySlot() const;
    int32 FindPartialStack(FName ItemId, int32 MaxStack) const;

    bool BuildPlaceTransform(AActor* User, const UItemDefinition* Def, FTransform& OutXform, FText& OutFailReason) const;
};