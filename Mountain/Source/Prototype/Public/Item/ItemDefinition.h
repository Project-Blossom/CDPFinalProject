#pragma once
#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
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
class Prototype_API UItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Material;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bStackable = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 99;
};