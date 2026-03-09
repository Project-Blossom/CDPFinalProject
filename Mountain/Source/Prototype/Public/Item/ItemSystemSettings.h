#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemSystemSettings.generated.h"

class UItemRegistryDataAsset;

UCLASS(Config = Game, DefaultConfig)
class PROTOTYPE_API UItemSystemSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    virtual FName GetContainerName() const override
    {
        return TEXT("Project");
    }

    virtual FName GetCategoryName() const override
    {
        return TEXT("Game");
    }

    virtual FName GetSectionName() const override
    {
        return TEXT("Item System");
    }

public:

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UItemRegistryDataAsset> ItemRegistry;
};