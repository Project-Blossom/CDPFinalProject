#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable,
    Equipment,
    Material,
    Quest,
    Tool
};

UCLASS(BlueprintType)
class PROTOTYPE_API UItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UItemDefinition();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bStackable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
    int32 MaxStack;
};