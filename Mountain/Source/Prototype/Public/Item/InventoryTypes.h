#pragma once
#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

class UItemDefinition;

USTRUCT(BlueprintType)
struct FItemStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<const UItemDefinition> Def = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    bool IsValid() const { return Def != nullptr && Count > 0; }
};