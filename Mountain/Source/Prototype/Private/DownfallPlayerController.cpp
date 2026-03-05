#include "DownfallPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"

#include "GameFramework/Pawn.h"
#include "TimerManager.h"

#include "Item/InventoryComponent.h"
#include "Item/InventoryWidget.h"
#include "Item/ItemDefinition.h"
#include "Blueprint/UserWidget.h"

UInventoryComponent* ADownfallPlayerController::GetInventoryFromPawn() const
{
    APawn* P = GetPawn();
    if (!P) return nullptr;

    return P->FindComponentByClass<UInventoryComponent>();
}

void ADownfallPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController()) return;

    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!Subsys) return;

    if (!PlayerMappingContext) return;

    Subsys->ClearAllMappings();
    Subsys->AddMappingContext(PlayerMappingContext, 0);
}

void ADownfallPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    GetWorldTimerManager().SetTimerForNextTick(this, &ADownfallPlayerController::DeferredInitAfterPossess);
}

void ADownfallPlayerController::DeferredInitAfterPossess()
{
    CreateAndBindInventoryUI();
    GiveTestItemsIfNeeded();
}

void ADownfallPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EIC)
    {
        return;
    }

    if (!UseItemAction)
        return;

    EIC->BindAction(UseItemAction, ETriggerEvent::Started, this, &ADownfallPlayerController::OnUseItemTriggered);
}

void ADownfallPlayerController::CreateAndBindInventoryUI()
{
    if (!IsLocalController()) return;
    if (!InventoryWidgetClass) return;
    if (InventoryWidget) return;

    UInventoryComponent* Inv = GetInventoryFromPawn();
    if (!Inv)
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ADownfallPlayerController::CreateAndBindInventoryUI);
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

    if (TestAnchorItemDef)
    {
        Inv->TryAddByDefinition(TestAnchorItemDef, 1);
    }
}

int32 ADownfallPlayerController::FindFirstUsableSlot(const UInventoryComponent* Inv) const
{
    if (!Inv) return INDEX_NONE;

    const TArray<FItemStack>& Slots = Inv->GetSlots();
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].IsValid())
            return i;
    }
    return INDEX_NONE;
}

void ADownfallPlayerController::OnUseItemTriggered(const FInputActionValue& Value)
{
    UInventoryComponent* Inv = GetInventoryFromPawn();
    APawn* P = GetPawn();
    if (!Inv || !P) return;

    const int32 Slot = FindFirstUsableSlot(Inv);
    if (Slot == INDEX_NONE) return;

    Inv->UseItem(Slot, P);
}