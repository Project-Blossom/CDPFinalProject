#include "DownfallPlayerController.h"

#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

#include "Item/InventoryComponent.h"
#include "Item/InventoryWidget.h"
#include "Item/ItemDefinition.h"

UInventoryComponent* ADownfallPlayerController::GetInventoryFromPawn() const
{
    APawn* P = GetPawn();
    if (!P)
    {
        return nullptr;
    }

    return P->FindComponentByClass<UInventoryComponent>();
}

void ADownfallPlayerController::BeginPlay()
{
    Super::BeginPlay();

    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

void ADownfallPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (GetWorld())
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ADownfallPlayerController::DeferredInitAfterPossess);
    }
}

void ADownfallPlayerController::DeferredInitAfterPossess()
{
    CreateAndBindInventoryUI();
    GiveTestItemsIfNeeded();

    if (UInventoryComponent* Inv = GetInventoryFromPawn())
    {
        Inv->SetPreviewEnabled(false);
    }
}

void ADownfallPlayerController::CreateAndBindInventoryUI()
{
    if (!IsLocalController()) return;
    if (!InventoryWidgetClass) return;
    if (InventoryWidget) return;

    UInventoryComponent* Inv = GetInventoryFromPawn();
    if (!Inv)
    {
        if (GetWorld())
        {
            GetWorldTimerManager().SetTimerForNextTick(this, &ADownfallPlayerController::CreateAndBindInventoryUI);
        }
        return;
    }

    InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
    if (!InventoryWidget) return;

    InventoryWidget->AddToViewport(10);
    InventoryWidget->BindInventory(Inv);
}

void ADownfallPlayerController::GiveTestItemsIfNeeded()
{
    UInventoryComponent* Inv = GetInventoryFromPawn();
    if (!Inv) return;
    if (!TestAnchorItemDef) return;

    Inv->TryAddByDefinition(TestAnchorItemDef, 1);
}