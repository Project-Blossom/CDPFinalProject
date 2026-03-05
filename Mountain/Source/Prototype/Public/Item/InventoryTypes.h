#pragma once
#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Placeable   UMETA(DisplayName = "Placeable"),   // 설치
    Consumable  UMETA(DisplayName = "Consumable"),  // 소모
    Weapon      UMETA(DisplayName = "Weapon"),      // 무기
};

UENUM(BlueprintType)
enum class EItemUseType : uint8
{
    None        UMETA(DisplayName = "None"),
    Consume     UMETA(DisplayName = "Consume"),
    Equip       UMETA(DisplayName = "Equip"),
    PlaceActor  UMETA(DisplayName = "PlaceActor"),
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

    // 무기 강화/내구도/랜덤옵션이 있을 때만 true
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