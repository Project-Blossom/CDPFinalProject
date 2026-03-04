#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemSubsystem.generated.h"

class UItemDefinition;

UCLASS()
class PROTOTYPE_API UItemSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TArray<TSoftObjectPtr<UItemDefinition>> ItemList;

    UFUNCTION(BlueprintCallable, Category = "Item")
    UItemDefinition* GetItemDefinitionById(FName ItemId);

private:
    UPROPERTY(Transient)
    TMap<FName, TObjectPtr<UItemDefinition>> Cache;

    void BuildCacheIfNeeded();
};