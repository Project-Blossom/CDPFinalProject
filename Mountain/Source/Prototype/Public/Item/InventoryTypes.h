#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

USTRUCT(BlueprintType)
struct FItemInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid InstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UpgradeLevel = 0;
};

USTRUCT(BlueprintType)
struct FItemStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasInstance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemInstanceData Instance;

    FORCEINLINE bool IsValid() const { return ItemId != NAME_None && Count > 0; }
    FORCEINLINE bool IsEmpty() const { return ItemId == NAME_None || Count <= 0; }

    void Reset()
    {
        ItemId = NAME_None;
        Count = 0;
        bHasInstance = false;
        Instance = FItemInstanceData();
    }
};