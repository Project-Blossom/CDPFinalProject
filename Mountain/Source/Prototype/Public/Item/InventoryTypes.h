#pragma once
#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Placeable   UMETA(DisplayName = "Placeable"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Weapon      UMETA(DisplayName = "Weapon"),
};

UENUM(BlueprintType)
enum class EItemUseType : uint8
{
    None                UMETA(DisplayName = "None"),
    Consume             UMETA(DisplayName = "Consume"),
    Equip               UMETA(DisplayName = "Equip"),
    UtilityEquip        UMETA(DisplayName = "UtilityEquip"),
    PlaceActor          UMETA(DisplayName = "PlaceActor"),
    AttachSafetyLine    UMETA(DisplayName = "AttachSafetyLine"),
    AttachAnchorToBolt  UMETA(DisplayName = "AttachAnchorToBolt"),
};

UENUM(BlueprintType)
enum class EConsumableEffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    RestoreStamina  UMETA(DisplayName = "RestoreStamina"),
};

UENUM(BlueprintType)
enum class EUtilityEffectType : uint8
{
    None             UMETA(DisplayName = "None"),
    ReduceInsanity   UMETA(DisplayName = "ReduceInsanity"),
};

USTRUCT(BlueprintType)
struct FItemInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FGuid InstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 UpgradeLevel = 0;
};

USTRUCT(BlueprintType)
struct FItemStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bHasInstance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FItemInstanceData Instance;

    FORCEINLINE bool IsEmpty() const { return (ItemId == NAME_None) || (Count <= 0); }
    FORCEINLINE bool IsValid() const { return !IsEmpty(); }

    FORCEINLINE void Reset()
    {
        ItemId = NAME_None;
        Count = 0;
        bHasInstance = false;
        Instance = FItemInstanceData();
    }
};