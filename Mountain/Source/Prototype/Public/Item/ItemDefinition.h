#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

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
    None        UMETA(DisplayName = "None"),
    Consume     UMETA(DisplayName = "Consume"),
    Equip       UMETA(DisplayName = "Equip"),   
    PlaceActor  UMETA(DisplayName = "PlaceActor"),
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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place", meta = (EditCondition = "UseType==EItemUseType::PlaceActor"))
    TSubclassOf<AActor> PlaceActorClass;
};