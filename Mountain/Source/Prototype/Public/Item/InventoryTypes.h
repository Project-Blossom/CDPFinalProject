#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "InventoryTypes.generated.h"

USTRUCT(BlueprintType)
struct FItemInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FGuid InstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    int32 Durability = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    int32 UpgradeLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    TMap<FName, int32> IntStats;

    bool IsValid() const { return InstanceId.IsValid(); }
};

USTRUCT(BlueprintType)
struct FItemStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FPrimaryAssetId ItemAssetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Count = 0;

    // 장비/무기/강화용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bHasInstance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (EditCondition = "bHasInstance"))
    FItemInstanceData Instance;

    bool IsEmpty() const { return !ItemAssetId.IsValid() || Count <= 0; }
    bool IsValid() const { return ItemAssetId.IsValid() && Count > 0; }

    void Reset()
    {
        ItemAssetId = FPrimaryAssetId();
        Count = 0;
        bHasInstance = false;
        Instance = FItemInstanceData{};
    }
};