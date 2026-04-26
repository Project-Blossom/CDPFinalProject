#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/InventoryTypes.h"
#include "ItemDefinition.generated.h"

class UTexture2D;
class AActor;

UCLASS(BlueprintType)
class PROTOTYPE_API UItemDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Consumable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Use")
    EItemUseType UseType = EItemUseType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consume")
    EConsumableEffectType ConsumableEffectType = EConsumableEffectType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consume", meta = (ClampMin = "0.0"))
    float ConsumableEffectValue = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility", meta = (EditCondition = "UseType==EItemUseType::UtilityEquip"))
    EUtilityEffectType UtilityEffectType = EUtilityEffectType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility", meta = (ClampMin = "0.0", EditCondition = "UseType==EItemUseType::UtilityEquip"))
    float UtilityEffectValue = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place", meta = (EditCondition = "UseType==EItemUseType::PlaceActor || UseType==EItemUseType::AttachAnchorToBolt"))
    TSubclassOf<AActor> PlaceActorClass;
};