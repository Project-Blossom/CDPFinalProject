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
    UserLocation  UMETA(DisplayName = "User / Player Location"),
    EventLocation UMETA(DisplayName = "Event / World Location")
};

USTRUCT(BlueprintType)
struct FItemSoundVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> Sound = nullptr;

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
    // - PitchMultiplier controls playback speed/pitch.
    // - PlaybackMode controls where the sound is heard from.
    //   Play2D: no distance attenuation.
    //   UserLocation: plays at the player/user location.
    //   EventLocation: plays at the actual action location, such as installed bolt/anchor position.
    // =====================================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    TObjectPtr<USoundBase> UseSound = nullptr;

    // 여기에 추가 후보를 넣으면 기본 UseSound까지 포함해서 동일 확률로 하나를 고른다.
    // 각 후보는 사운드와 PitchMultiplier를 따로 가진다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    TArray<FItemSoundVariant> UseSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float UseSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    EItemSoundPlaybackMode UseSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;

    // 이전 버전 호환용. 새 설정은 UseSoundPlaybackMode를 사용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Use")
    bool bUseSoundAtUserLocation = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TObjectPtr<USoundBase> EquipSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TArray<FItemSoundVariant> EquipSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float EquipSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    EItemSoundPlaybackMode EquipSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TObjectPtr<USoundBase> UnequipSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    TArray<FItemSoundVariant> UnequipSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float UnequipSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Equip")
    EItemSoundPlaybackMode UnequipSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;

    // 볼트 설치 / 앵커를 볼트에 설치할 때 사용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    TObjectPtr<USoundBase> PlaceSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    TArray<FItemSoundVariant> PlaceSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float PlaceSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Place")
    EItemSoundPlaybackMode PlaceSoundPlaybackMode = EItemSoundPlaybackMode::EventLocation;

    // 앵커 로프가 실제로 발동되는 순간에 사용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    TObjectPtr<USoundBase> ActivateSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    TArray<FItemSoundVariant> ActivateSoundVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float ActivateSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Activate")
    EItemSoundPlaybackMode ActivateSoundPlaybackMode = EItemSoundPlaybackMode::EventLocation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place", meta = (EditCondition = "UseType==EItemUseType::PlaceActor || UseType==EItemUseType::AttachAnchorToBolt"))
    TSubclassOf<AActor> PlaceActorClass;
};
