#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/InventoryTypes.h"
#include "ItemDefinition.generated.h"

class UTexture2D;
class USoundBase;
class AActor;

UENUM(BlueprintType)
enum class EItemSoundPlaybackMode : uint8
{
    Play2D        UMETA(DisplayName = "2D"),
    UserLocation  UMETA(DisplayName = "Player"),
    EventLocation UMETA(DisplayName = "World")
};

USTRUCT(BlueprintType)
struct FItemSoundVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float PitchMultiplier = 1.0f;
};

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    TArray<FItemSoundVariant> UseSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float UseSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float UseSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    EItemSoundPlaybackMode UseSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TObjectPtr<USoundBase> EquipSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TArray<FItemSoundVariant> EquipSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float EquipSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float EquipSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    EItemSoundPlaybackMode EquipSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TObjectPtr<USoundBase> UnequipSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TArray<FItemSoundVariant> UnequipSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float UnequipSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float UnequipSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    EItemSoundPlaybackMode UnequipSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    TObjectPtr<USoundBase> PlaceSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    TArray<FItemSoundVariant> PlaceSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float PlaceSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float PlaceSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    EItemSoundPlaybackMode PlaceSoundPlaybackMode = EItemSoundPlaybackMode::EventLocation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    TObjectPtr<USoundBase> ActivateSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    TArray<FItemSoundVariant> ActivateSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float ActivateSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float ActivateSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    EItemSoundPlaybackMode ActivateSoundPlaybackMode = EItemSoundPlaybackMode::EventLocation;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Anchor Retrieve")
    TObjectPtr<USoundBase> AnchorRetrieveSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Anchor Retrieve")
    TArray<FItemSoundVariant> AnchorRetrieveSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Anchor Retrieve", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float AnchorRetrieveSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Anchor Retrieve", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float AnchorRetrieveSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Anchor Retrieve")
    EItemSoundPlaybackMode AnchorRetrieveSoundPlaybackMode = EItemSoundPlaybackMode::EventLocation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place", meta = (EditCondition = "UseType==EItemUseType::PlaceActor || UseType==EItemUseType::AttachAnchorToBolt"))
    TSubclassOf<AActor> PlaceActorClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place|Preview", meta = (EditCondition = "UseType==EItemUseType::PlaceActor || UseType==EItemUseType::AttachAnchorToBolt"))
    TSubclassOf<AActor> PreviewActorClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place|Restriction", meta = (EditCondition = "UseType==EItemUseType::PlaceActor"))
    bool bPlaceOnlyOnCliffSurface = true;
};