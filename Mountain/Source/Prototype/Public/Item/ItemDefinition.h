#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/InventoryTypes.h"
#include "ItemDefinition.generated.h"

class UTexture2D;
class USoundBase;
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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility")
    EUtilityEffectType UtilityEffectType = EUtilityEffectType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility", meta = (ClampMin = "0.0"))
    float UtilityEffectValue = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility|Cooldown", meta = (ClampMin = "0.0"))
    float UtilityCooldownSeconds = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility|Lamp")
    bool bBlockMonsterSenseOnUtilityUse = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility|Lamp", meta = (ClampMin = "0.0"))
    float MonsterSenseBlockDuration = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTexture2D> Icon = nullptr;

    // =====================================================
    // Sound
    // =====================================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    TObjectPtr<USoundBase> UseSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float UseSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    bool bUseSoundAtUserLocation = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TObjectPtr<USoundBase> EquipSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float EquipSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TObjectPtr<USoundBase> UnequipSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float UnequipSoundPitchMultiplier = 1.0f;

    // 볼트 설치 / 앵커를 볼트에 설치할 때 사용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    TObjectPtr<USoundBase> PlaceSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float PlaceSoundPitchMultiplier = 1.0f;

    // 앵커 로프가 실제로 발동되는 순간에 사용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    TObjectPtr<USoundBase> ActivateSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float ActivateSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place", meta = (EditCondition = "UseType==EItemUseType::PlaceActor || UseType==EItemUseType::AttachAnchorToBolt"))
    TSubclassOf<AActor> PlaceActorClass;
};