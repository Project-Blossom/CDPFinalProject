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
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Item")
    UItemDefinition* GetItemDefinitionById(FName ItemId) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TArray<TObjectPtr<UItemDefinition>> ItemList;

private:
    UPROPERTY(Transient)
    TMap<FName, TObjectPtr<UItemDefinition>> ItemMap;
};