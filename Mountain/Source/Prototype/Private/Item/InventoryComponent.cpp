#include "Item/InventoryComponent.h"

#include "Item/ItemSubsystem.h"
#include "Item/ItemDefinition.h"
#include "Item/InventorySaveGame.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    Slots.SetNum(SlotCount);
    OnInventoryChanged.Broadcast();

    if (bPreviewEnabled && ShouldRunPreview())
    {
        EnsurePreviewActor();
    }
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyPreviewActor();
    Super::EndPlay(EndPlayReason);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bPreviewEnabled) return;
    if (!ShouldRunPreview()) return;

    if (PreviewUpdateInterval > 0.f)
    {
        PreviewAccum += DeltaTime;
        if (PreviewAccum < PreviewUpdateInterval) return;
        PreviewAccum = 0.f;
    }

    UpdatePreview(DeltaTime);
}

// =========================================================
// Preview API
// =========================================================

void UInventoryComponent::SetPreviewEnabled(bool bEnabled)
{
    bPreviewEnabled = bEnabled;

    if (!bPreviewEnabled)
    {
        DestroyPreviewActor();
        bLastPreviewValid = false;
        LastPreviewReason = FText::FromString(TEXT("Preview disabled"));
        return;
    }

    if (ShouldRunPreview())
    {
        EnsurePreviewActor();
    }
}

void UInventoryComponent::SetPreviewSlotIndex(int32 NewIndex)
{
    PreviewSlotIndex = NewIndex;
}

void UInventoryComponent::SetPreviewActorClass(TSubclassOf<AActor> InClass)
{
    DefaultPreviewActorClass = InClass;

    DestroyPreviewActor();

    if (bPreviewEnabled && ShouldRunPreview())
    {
        EnsurePreviewActor();
    }
}

bool UInventoryComponent::GetLastPreviewState(bool& bOutValid, FText& OutReason) const
{
    bOutValid = bLastPreviewValid;
    OutReason = LastPreviewReason;
    return true;
}

// =========================================================
// Preview internals
// =========================================================

bool UInventoryComponent::ShouldRunPreview() const
{
    const APawn* PawnOwner = Cast<APawn>(GetOwner());
    if (!PawnOwner) return false;

    const APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
    if (!PC) return false;

    return PC->IsLocalController();
}

AActor* UInventoryComponent::GetPreviewUserActor() const
{
    return GetOwner();
}

void UInventoryComponent::EnsurePreviewActor()
{
    if (PreviewActor) return;

    UWorld* W = GetWorld();
    if (!W) return;

    if (!DefaultPreviewActorClass)
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PreviewActor = W->SpawnActor<AActor>(DefaultPreviewActorClass, FTransform::Identity, Params);
    if (!PreviewActor) return;

    PreviewActor->SetReplicates(false);
    PreviewActor->SetReplicateMovement(false);
    PreviewActor->SetActorEnableCollision(false);
    PreviewActor->SetActorHiddenInGame(true);
}

void UInventoryComponent::DestroyPreviewActor()
{
    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
}

void UInventoryComponent::UpdatePreview(float)
{
    EnsurePreviewActor();
    if (!PreviewActor)
    {
        bLastPreviewValid = false;
        LastPreviewReason = FText::FromString(TEXT("No preview actor class"));
        return;
    }

    AActor* User = GetPreviewUserActor();
    if (!User)
    {
        PreviewActor->SetActorHiddenInGame(true);
        bLastPreviewValid = false;
        LastPreviewReason = FText::FromString(TEXT("No user"));
        return;
    }

    FTransform Xf;
    FText Fail;
    const bool bOK = ComputePreviewTransform(PreviewSlotIndex, User, Xf, Fail);

    bLastPreviewValid = bOK;
    LastPreviewReason = bOK ? FText::GetEmpty() : Fail;

    if (bOK)
    {
        PreviewActor->SetActorTransform(Xf);
        PreviewActor->SetActorHiddenInGame(false);
    }
    else
    {
        PreviewActor->SetActorHiddenInGame(true);
    }
}

bool UInventoryComponent::ComputePreviewTransform(int32 Index, AActor* User, FTransform& OutXform, FText& OutFailReason) const
{
    if (!Slots.IsValidIndex(Index) || !User)
    {
        OutFailReason = FText::FromString(TEXT("Invalid slot/user"));
        return false;
    }

    const FItemStack& S = Slots[Index];
    if (!S.IsValid())
    {
        OutFailReason = FText::FromString(TEXT("Empty slot"));
        return false;
    }

    UItemSubsystem* IS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
    const UItemDefinition* Def = IS ? IS->GetItemDefinitionById(S.ItemId) : nullptr;
    if (!Def)
    {
        OutFailReason = FText::FromString(TEXT("No definition"));
        return false;
    }

    if (Def->UseType == EItemUseType::PlaceActor)
    {
        return BuildPlaceTransform(User, Def, OutXform, OutFailReason);
    }

    OutFailReason = FText::FromString(TEXT("No preview for this item type"));
    return false;
}

// =========================================================
// Inventory core
// =========================================================

int32 UInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].IsEmpty())
            return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindPartialStack(FName ItemId, int32 MaxStack) const
{
    if (ItemId == NAME_None || MaxStack <= 1) return INDEX_NONE;

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const FItemStack& S = Slots[i];
        if (S.IsValid() && S.ItemId == ItemId && !S.bHasInstance && S.Count < MaxStack)
            return i;
    }
    return INDEX_NONE;
}

bool UInventoryComponent::TryAdd(FName ItemId, int32 Count, bool bForceInstance)
{
    if (ItemId == NAME_None || Count <= 0) return false;

    UItemSubsystem* IS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
    const UItemDefinition* Def = IS ? IS->GetItemDefinitionById(ItemId) : nullptr;

    const int32 MaxStack = Def ? FMath::Max(1, Def->MaxStack) : 1;

    const bool bUniqueSlot = bForceInstance || (MaxStack == 1);
    const bool bShouldInstance = bForceInstance || (Def && Def->UseType == EItemUseType::Equip);

    int32 Remaining = Count;

    // 1) 스택형이면 기존 스택 채우기
    if (!bUniqueSlot && MaxStack > 1)
    {
        while (Remaining > 0)
        {
            const int32 Idx = FindPartialStack(ItemId, MaxStack);
            if (Idx == INDEX_NONE) break;

            const int32 CanAdd = MaxStack - Slots[Idx].Count;
            const int32 AddNow = FMath::Min(CanAdd, Remaining);
            Slots[Idx].Count += AddNow;
            Remaining -= AddNow;
        }
    }

    // 2) 빈 슬롯에 새로 생성
    while (Remaining > 0)
    {
        const int32 Empty = FindEmptySlot();
        if (Empty == INDEX_NONE) break;

        FItemStack& S = Slots[Empty];
        S.Reset();
        S.ItemId = ItemId;

        S.Count = bUniqueSlot ? 1 : FMath::Min(MaxStack, Remaining);
        Remaining -= S.Count;

        if (bShouldInstance)
        {
            S.bHasInstance = true;
            S.Instance.InstanceId = FGuid::NewGuid();
            S.Instance.UpgradeLevel = 0;
            S.Count = 1;
        }
    }

    OnInventoryChanged.Broadcast();
    return (Remaining == 0);
}

bool UInventoryComponent::TryAddByDefinition(const UItemDefinition* Def, int32 Count, bool bForceInstance)
{
    if (!Def || Count <= 0) return false;
    return TryAdd(Def->ItemId, Count, bForceInstance);
}

bool UInventoryComponent::TryRemove(FName ItemId, int32 Count)
{
    if (ItemId == NAME_None || Count <= 0) return false;

    int32 Remaining = Count;

    for (int32 i = Slots.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        FItemStack& S = Slots[i];
        if (!S.IsValid() || S.ItemId != ItemId) continue;

        const int32 Take = FMath::Min(S.Count, Remaining);
        S.Count -= Take;
        Remaining -= Take;

        if (S.Count <= 0) S.Reset();
    }

    OnInventoryChanged.Broadcast();
    return (Remaining == 0);
}

bool UInventoryComponent::BuildPlaceTransform(AActor* User, const UItemDefinition* Def, FTransform& OutXform, FText& OutFailReason) const
{
    if (!User)
    {
        OutFailReason = FText::FromString(TEXT("Invalid user"));
        return false;
    }

    if (!Def || Def->UseType != EItemUseType::PlaceActor)
    {
        OutFailReason = FText::FromString(TEXT("Item cannot be placed"));
        return false;
    }

    if (!Def->PlaceActorClass)
    {
        OutFailReason = FText::FromString(TEXT("No actor assigned for placement"));
        return false;
    }

    // ---- 카메라 기준 라인트레이스 ----
    FVector ViewLoc = User->GetActorLocation();
    FRotator ViewRot = User->GetActorRotation();

    if (const APawn* P = Cast<APawn>(User))
    {
        if (APlayerController* PC = Cast<APlayerController>(P->GetController()))
        {
            PC->GetPlayerViewPoint(ViewLoc, ViewRot);
        }
    }

    const FVector Start = ViewLoc;
    const FVector Dir = ViewRot.Vector();
    const FVector End = Start + Dir * PlaceTraceDistanceCm;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PlaceTrace), false, User);
    Params.bReturnPhysicalMaterial = false;

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    if (!bHit || !Hit.bBlockingHit)
    {
        OutFailReason = FText::FromString(TEXT("No valid surface to place the item"));
        return false;
    }

    const float Dist = FVector::Dist(Start, Hit.ImpactPoint);
    if (Dist > PlaceRangeCm)
    {
        OutFailReason = FText::FromString(TEXT("Target is too far away"));
        return false;
    }

    // ---- 법선 정렬 ----
    const FVector Normal = Hit.ImpactNormal.GetSafeNormal();
    const FVector Forward = -Normal;
    const FRotator Rot = FRotationMatrix::MakeFromX(Forward).Rotator();

    const FVector Pos = Hit.ImpactPoint + Normal * (-PlaceEmbedCm);

    OutXform = FTransform(Rot, Pos);
    return true;
}

bool UInventoryComponent::UseItem(int32 Index, AActor* User)
{
    if (!Slots.IsValidIndex(Index) || !User) return false;

    FItemStack& S = Slots[Index];
    if (!S.IsValid()) return false;

    UItemSubsystem* IS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
    UItemDefinition* Def = IS ? IS->GetItemDefinitionById(S.ItemId) : nullptr;
    if (!Def) return false;

    switch (Def->UseType)
    {
    case EItemUseType::Consume:
    {
        BP_OnConsume(User, Def, 1);

        S.Count -= 1;
        if (S.Count <= 0) S.Reset();

        OnInventoryChanged.Broadcast();
        return true;
    }

    case EItemUseType::PlaceActor:
    {
        FTransform SpawnXform;
        FText Fail;
        if (!BuildPlaceTransform(User, Def, SpawnXform, Fail))
        {
            BP_OnUseFailed(User, Fail);
            return false;
        }

        const bool bPlaced = BP_OnPlace(User, Def, SpawnXform);
        if (!bPlaced)
        {
            BP_OnUseFailed(User, FText::FromString(TEXT("Failed to place the item")));
            return false;
        }

        S.Count -= 1;
        if (S.Count <= 0) S.Reset();

        OnInventoryChanged.Broadcast();
        return true;
    }

    case EItemUseType::Equip:
    {
        if (!S.bHasInstance)
        {
            S.bHasInstance = true;
            S.Instance.InstanceId = FGuid::NewGuid();
            S.Instance.UpgradeLevel = 0;
            S.Count = 1;
        }

        BP_OnEquip(User, Def, S.Instance);
        return true;
    }

    default:
        return false;
    }
}

bool UInventoryComponent::TransferTo(UInventoryComponent* Target, int32 FromIndex, int32 Count)
{
    if (!Target) return false;
    if (!Slots.IsValidIndex(FromIndex)) return false;
    if (Count <= 0) return false;

    FItemStack& From = Slots[FromIndex];
    if (!From.IsValid()) return false;

    const int32 MoveCount = FMath::Min(From.Count, Count);

    if (From.bHasInstance && MoveCount != 1) return false;

    const bool bAdded = Target->TryAdd(From.ItemId, MoveCount, From.bHasInstance);
    if (!bAdded) return false;

    From.Count -= MoveCount;
    if (From.Count <= 0) From.Reset();

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
    UInventorySaveGame* SaveObj = Cast<UInventorySaveGame>(UGameplayStatics::CreateSaveGameObject(UInventorySaveGame::StaticClass()));
    if (!SaveObj) return false;

    SaveObj->SlotCount = SlotCount;
    SaveObj->Slots = Slots;

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

bool UInventoryComponent::LoadFromSlot(const FString& SlotName, int32 UserIndex)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)) return false;

    UInventorySaveGame* SaveObj = Cast<UInventorySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!SaveObj) return false;

    SlotCount = FMath::Max(1, SaveObj->SlotCount);
    Slots = SaveObj->Slots;
    Slots.SetNum(SlotCount);

    OnInventoryChanged.Broadcast();
    return true;
}