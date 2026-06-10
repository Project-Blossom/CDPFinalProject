#include "Item/ItemSpawner.h"

#include "Item/ItemDropActor.h"
#include "MountainGenWorldActor.h"

#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AItemSpawner::AItemSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DefaultDropActorClass = AItemDropActor::StaticClass();
}

void AItemSpawner::BeginPlay()
{
    Super::BeginPlay();

    ResolveMountain();

    if (TargetMountain && bSpawnOnlyAfterMountainGenerated)
    {
        TargetMountain->OnMountainGenerated.RemoveDynamic(this, &AItemSpawner::HandleMountainGenerated);
        TargetMountain->OnMountainGenerated.AddDynamic(this, &AItemSpawner::HandleMountainGenerated);

        if (bAutoSpawnOnBeginPlay && TargetMountain->HasGeneratedMesh())
        {
            SpawnItems();
        }
        else if (bAutoSpawnOnBeginPlay && GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                DeferredInitialSpawnTimer,
                this,
                &AItemSpawner::TryInitialSpawnFallback,
                0.25f,
                false
            );
        }
    }
    else if (bAutoSpawnOnBeginPlay)
    {
        SpawnItems();
    }
}

void AItemSpawner::TryInitialSpawnFallback()
{
    if (!TargetMountain)
    {
        ResolveMountain();
    }

    if (TargetMountain && TargetMountain->HasGeneratedMesh() && SpawnedDrops.Num() == 0)
    {
        SpawnItems();
    }
}

void AItemSpawner::HandleMountainGenerated(AActor* Generator)
{
    if (!Generator || Generator != TargetMountain.Get())
    {
        return;
    }

    if (!bRespawnWhenMountainRegenerates && SpawnedDrops.Num() > 0)
    {
        return;
    }

    SpawnItems();
}

bool AItemSpawner::ResolveMountain()
{
    if (TargetMountain)
    {
        return true;
    }

    if (!bAutoFindMountain || !GetWorld())
    {
        return false;
    }

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMountainGenWorldActor::StaticClass(), Found);

    if (Found.Num() > 0)
    {
        TargetMountain = Cast<AMountainGenWorldActor>(Found[0]);
    }

    return TargetMountain != nullptr;
}

bool AItemSpawner::GetMountainBounds(FBox& OutBounds) const
{
    OutBounds = FBox(EForceInit::ForceInit);

    if (!TargetMountain || !TargetMountain->HasGeneratedMesh())
    {
        return false;
    }

    OutBounds = TargetMountain->GetGeneratedWorldBounds();
    return OutBounds.IsValid != 0;
}

bool AItemSpawner::TraceFrontSurfaceAtYZ(float Y, float Z, FHitResult& OutHit) const
{
    if (!GetWorld() || !TargetMountain)
    {
        return false;
    }

    const float FrontX = TargetMountain->GetFrontSurfaceWorldX();

    // +X 앞쪽에서 -X 방향으로 쏴서 실제 노이즈가 반영된 절벽 앞면을 찾는다.
    const FVector Start(FrontX + FrontTraceStartDistanceCm, Y, Z);
    const FVector End(FrontX - FrontTraceBackDistanceCm, Y, Z);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(ItemSpawnerFrontSurfaceTrace), false);
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        return false;
    }

    if (Hit.GetActor() != TargetMountain)
    {
        return false;
    }

    const FVector SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();
    if (FVector::DotProduct(SurfaceNormal, FVector::ForwardVector) < FrontFacingNormalDotMin)
    {
        return false;
    }

    OutHit = Hit;
    return true;
}

int32 AItemSpawner::GetTotalRequestedDropCount() const
{
    int32 Total = 0;

    for (const FCliffItemSpawnEntry& Entry : CliffItemsToSpawn)
    {
        if (Entry.ItemId != NAME_None && Entry.SpawnCount > 0)
        {
            Total += Entry.SpawnCount;
        }
    }

    return Total;
}

float AItemSpawner::CalculateSurfaceDistance(const FCliffItemSpawnEntry& Entry) const
{
    if (Entry.SurfaceDistanceOverrideCm > 0.0f)
    {
        return Entry.SurfaceDistanceOverrideCm;
    }

    float Distance = SurfaceDistanceFromCliffCm;

    if (bAddVisualMeshRadiusToDistance && Entry.VisualMesh)
    {
        Distance += Entry.VisualMesh->GetBounds().SphereRadius + MeshClearancePaddingCm;
    }

    return FMath::Max(0.0f, Distance);
}

bool AItemSpawner::BuildSpawnTransform(int32 LinearIndex, int32 TotalCount, const FCliffItemSpawnEntry& Entry, FTransform& OutTransform) const
{
    FBox Bounds;
    if (!GetMountainBounds(Bounds) || !TargetMountain || TotalCount <= 0)
    {
        return false;
    }

    const int32 ColumnCount = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt((float)TotalCount)));
    const int32 RowCount = FMath::Max(1, FMath::CeilToInt((float)TotalCount / (float)ColumnCount));

    const int32 Column = LinearIndex % ColumnCount;
    const int32 Row = LinearIndex / ColumnCount;

    const float YMinRaw = Bounds.Min.Y + SideMarginCm;
    const float YMaxRaw = Bounds.Max.Y - SideMarginCm;
    const float YMin = (YMinRaw < YMaxRaw) ? YMinRaw : Bounds.Min.Y;
    const float YMax = (YMinRaw < YMaxRaw) ? YMaxRaw : Bounds.Max.Y;

    const float HeightAlphaMin = FMath::Clamp(FMath::Min(SpawnHeightRatioMin, SpawnHeightRatioMax), 0.0f, 1.0f);
    const float HeightAlphaMax = FMath::Clamp(FMath::Max(SpawnHeightRatioMin, SpawnHeightRatioMax), 0.0f, 1.0f);
    const float ZMin = FMath::Lerp(Bounds.Min.Z, Bounds.Max.Z, HeightAlphaMin);
    const float ZMax = FMath::Lerp(Bounds.Min.Z, Bounds.Max.Z, HeightAlphaMax);

    const float YAlpha = ((float)Column + 0.5f) / (float)ColumnCount;
    const float ZAlpha = ((float)Row + 0.5f) / (float)RowCount;

    const float Y = FMath::Lerp(YMin, YMax, YAlpha);
    const float Z = FMath::Lerp(ZMin, ZMax, ZAlpha);

    FHitResult SurfaceHit;
    if (!TraceFrontSurfaceAtYZ(Y, Z, SurfaceHit))
    {
        return false;
    }

    const FVector SurfaceNormal = SurfaceHit.ImpactNormal.GetSafeNormal();

    // 아이템 배치 기준:
    // 완성된 절벽 노이즈 앞면 SurfaceHit.ImpactPoint에서 표면 노말 방향으로 1m.
    const FVector SpawnLocation = SurfaceHit.ImpactPoint + SurfaceNormal * CalculateSurfaceDistance(Entry);

    OutTransform = FTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);

    if (bDrawDebugSpawnPoints && GetWorld())
    {
        DrawDebugSphere(GetWorld(), SurfaceHit.ImpactPoint, 25.0f, 12, FColor::Yellow, false, 10.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), SpawnLocation, 35.0f, 12, FColor::Cyan, false, 10.0f, 0, 2.0f);
        DrawDebugLine(GetWorld(), SurfaceHit.ImpactPoint, SpawnLocation, FColor::Cyan, false, 10.0f, 0, 2.0f);
    }

    return true;
}

void AItemSpawner::SpawnItems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (!ResolveMountain())
    {
        if (bVerboseLog)
        {
            UE_LOG(LogTemp, Error, TEXT("ItemSpawner '%s': no MountainGenWorldActor found"), *GetName());
        }
        return;
    }

    if (!TargetMountain->HasGeneratedMesh())
    {
        if (bVerboseLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("ItemSpawner '%s': mountain mesh is not ready"), *GetName());
        }
        return;
    }

    if (bClearExistingDropsBeforeSpawn)
    {
        ClearSpawnedItems();
    }

    const int32 TotalCount = GetTotalRequestedDropCount();
    if (TotalCount <= 0)
    {
        return;
    }

    int32 LinearIndex = 0;
    int32 SpawnedCount = 0;

    for (const FCliffItemSpawnEntry& Entry : CliffItemsToSpawn)
    {
        if (Entry.ItemId == NAME_None || Entry.SpawnCount <= 0)
        {
            continue;
        }

        TSubclassOf<AItemDropActor> ClassToSpawn = Entry.DropActorClass ? Entry.DropActorClass : DefaultDropActorClass;
        if (!ClassToSpawn)
        {
            if (bVerboseLog)
            {
                UE_LOG(LogTemp, Warning, TEXT("ItemSpawner '%s': no drop actor class for item %s"),
                    *GetName(), *Entry.ItemId.ToString());
            }

            LinearIndex += Entry.SpawnCount;
            continue;
        }

        for (int32 i = 0; i < Entry.SpawnCount; ++i)
        {
            FTransform SpawnTransform;
            if (!BuildSpawnTransform(LinearIndex, TotalCount, Entry, SpawnTransform))
            {
                if (bVerboseLog)
                {
                    UE_LOG(LogTemp, Warning, TEXT("ItemSpawner '%s': failed to place %s at cliff front slot %d/%d"),
                        *GetName(), *Entry.ItemId.ToString(), LinearIndex + 1, TotalCount);
                }

                ++LinearIndex;
                continue;
            }

            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AItemDropActor* Drop = World->SpawnActor<AItemDropActor>(ClassToSpawn, SpawnTransform, Params);
            if (Drop)
            {
                Drop->InitializeDropWithMesh(Entry.ItemId, Entry.CountPerDrop, Entry.VisualMesh);
                SpawnedDrops.Add(Drop);
                ++SpawnedCount;
            }

            ++LinearIndex;
        }
    }

    if (bVerboseLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemSpawner '%s': spawned %d/%d item drops 1m from noisy cliff front"),
            *GetName(), SpawnedCount, TotalCount);
    }
}

void AItemSpawner::ClearSpawnedItems()
{
    for (AItemDropActor* Drop : SpawnedDrops)
    {
        if (IsValid(Drop))
        {
            Drop->Destroy();
        }
    }

    SpawnedDrops.Reset();
}