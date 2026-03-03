#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/AssetManager.h"
#include "ItemSubsystem.generated.h"

class UItemDefinition;

UCLASS()
class PROTOTYPE_API UItemSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Item")
    UItemDefinition* GetItemDefinition(const FPrimaryAssetId& Id) const;

    UFUNCTION(BlueprintCallable, Category = "Item")
    static FPrimaryAssetId MakeItemId(FName ItemId)
    {
        return FPrimaryAssetId(TEXT("Item"), ItemId);
    }
};