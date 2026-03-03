#include "Item/InventoryWidget.h"

#include "Components/TextBlock.h"
#include "Item/InventoryComponent.h"
#include "Item/InventoryTypes.h"

#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    Refresh();
}

void UInventoryWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UInventoryWidget::BindInventory(UInventoryComponent* InInventory)
{
    if (Inventory == InInventory)
    {
        Refresh();
        return;
    }

    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Inventory = InInventory;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::Refresh);
    }

    Refresh();
}

void UInventoryWidget::Refresh()
{
    if (!SlotsText)
    {
        return;
    }

    if (!Inventory)
    {
        SlotsText->SetText(FText::FromString(TEXT("Inventory: (not bound)")));
        return;
    }

    UItemSubsystem* IS = nullptr;
    if (UWorld* W = GetWorld())
    {
        if (UGameInstance* GI = W->GetGameInstance())
        {
            IS = GI->GetSubsystem<UItemSubsystem>();
        }
    }

    const TArray<FItemStack>& Slots = Inventory->GetSlots();

    FString Out;
    Out += FString::Printf(TEXT("Inventory (%d slots)\n"), Slots.Num());
    Out += TEXT("----------------------\n");

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const FItemStack& S = Slots[i];

        if (S.IsEmpty())
        {
            Out += FString::Printf(TEXT("[%02d] (empty)\n"), i);
            continue;
        }

        UItemDefinition* Def = IS ? IS->GetItemDefinition(S.ItemAssetId) : nullptr;

        const FString Name = Def ? Def->DisplayName.ToString() : S.ItemAssetId.ToString();

        if (S.bHasInstance)
        {
            Out += FString::Printf(TEXT("[%02d] %s x%d  (Inst:%s, Upg:%d)\n"),
                i, *Name, S.Count,
                *S.Instance.InstanceId.ToString(EGuidFormats::Short),
                S.Instance.UpgradeLevel);
        }
        else
        {
            Out += FString::Printf(TEXT("[%02d] %s x%d\n"), i, *Name, S.Count);
        }
    }

    SlotsText->SetText(FText::FromString(Out));
}