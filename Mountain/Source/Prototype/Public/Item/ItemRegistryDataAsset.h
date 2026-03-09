#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemRegistryDataAsset.generated.h"

class UItemDefinition;

UCLASS(BlueprintType)
class PROTOTYPE_API UItemRegistryDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TArray<TSoftObjectPtr<UItemDefinition>> Items;

};