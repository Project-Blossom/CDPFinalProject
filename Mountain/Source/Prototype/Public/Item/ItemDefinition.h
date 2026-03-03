#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Anchor      UMETA(DisplayName = "Anchor"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Equipment   UMETA(DisplayName = "Equipment"),
    Etc         UMETA(DisplayName = "Etc"),
};

UENUM(BlueprintType)
enum class EItemUseType : uint8
{
    None        UMETA(DisplayName = "None"),
    Consume     UMETA(DisplayName = "Consume"),      // 회복/버프
    PlaceActor  UMETA(DisplayName = "PlaceActor"),   // 설치형
    Equip       UMETA(DisplayName = "Equip"),        // 무기/장비
};

UCLASS(BlueprintType)
class PROTOTYPE_API UItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Etc;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Use")
    EItemUseType UseType = EItemUseType::None;

    // ---------- UI ----------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTexture2D> Icon = nullptr;

    // ---------- 설치형 ----------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Place")
    TSubclassOf<AActor> PlaceActorClass = nullptr;

    // ---------- 소모형 ----------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consume")
    float HealAmount = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consume")
    float StaminaAmount = 0.f;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        // 타입: "Item", 이름: ItemId
        return FPrimaryAssetId(TEXT("Item"), ItemId);
    }
};