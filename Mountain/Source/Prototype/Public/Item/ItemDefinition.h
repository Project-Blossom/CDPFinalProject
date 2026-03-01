#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Anchor      UMETA(DisplayName = "Anchor"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Etc         UMETA(DisplayName = "Etc"),
};

UCLASS(BlueprintType)
class PROTOTYPE_API UItemDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    UItemDefinition();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Etc;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;
};