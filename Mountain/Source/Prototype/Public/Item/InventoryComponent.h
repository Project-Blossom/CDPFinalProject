#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/InventoryTypes.h"
#include "InventoryComponent.generated.h"

class UItemDefinition;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPE_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 SlotCount = 25;

    // ---------- Áß¾Ó ¸Ç¼Õ ½½·Ô ----------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0"))
    int32 ReservedCenterSlotIndex = 12;

    // ---------- Place settings ----------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Place", meta = (ClampMin = "1.0"))
    float PlaceRangeCm = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Place", meta = (ClampMin = "1.0"))
    float PlaceTraceDistanceCm = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Place", meta = (ClampMin = "0.0"))
    float PlaceEmbedCm = 5.f;

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FItemStack>& GetSlots() const { return Slots; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasValidItemAt(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    UItemDefinition* GetItemDefinitionAt(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    UTexture2D* GetItemIconAt(int32 Index) const;

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

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetReservedCenterSlotIndex() const { return ReservedCenterSlotIndex; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsReservedCenterSlot(int32 Index) const { return Index == ReservedCenterSlotIndex; }

    // ---- Save/Load ----
    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    bool SaveToSlot(const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    bool LoadFromSlot(const FString& SlotName, int32 UserIndex = 0);

    // =========================================================
    // Preview API
    // =========================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    void SetPreviewEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    bool IsPreviewEnabled() const { return bPreviewEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    void SetPreviewSlotIndex(int32 NewIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    int32 GetPreviewSlotIndex() const { return PreviewSlotIndex; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    void SetPreviewActorClass(TSubclassOf<AActor> InClass);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    AActor* GetPreviewActor() const { return PreviewActor; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preview")
    bool GetLastPreviewState(bool& bOutValid, FText& OutReason) const;

protected:
    // ----- BP hooks -----
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

    // ----------------- Preview internals -----------------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AActor> DefaultPreviewActorClass;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> PreviewActor;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true"))
    bool bPreviewEnabled = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
    int32 PreviewSlotIndex = 0;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true"))
    bool bLastPreviewValid = false;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true"))
    FText LastPreviewReason;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Preview", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
    float PreviewUpdateInterval = 0.0f;

    float PreviewAccum = 0.0f;

private:
    int32 FindEmptySlot() const;
    int32 FindPartialStack(FName ItemId, int32 MaxStack) const;

    bool BuildPlaceTransform(AActor* User, const UItemDefinition* Def, FTransform& OutXform, FText& OutFailReason) const;
    bool BuildAttachAnchorPreviewTransform(AActor* User, const UItemDefinition* Def, FTransform& OutXform, FText& OutFailReason) const;

    void SanitizeReservedCenterSlot();

    // ---- preview helpers ----
    bool ShouldRunPreview() const;
    AActor* GetPreviewUserActor() const;
    void EnsurePreviewActor();
    void DestroyPreviewActor();
    void UpdatePreview(float DeltaTime);
    bool ComputePreviewTransform(int32 Index, AActor* User, FTransform& OutXform, FText& OutFailReason) const;
};