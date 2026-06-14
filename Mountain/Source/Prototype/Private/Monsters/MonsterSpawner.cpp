#include "Monsters/MonsterSpawner.h"

#include "MountainGenWorldActor.h"

#include "Monsters/MonsterBase.h"
#include "Monsters/WallCrawler.h"
#include "Monsters/FlyingMonster.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/FlyingAttacker.h"
#include "Monsters/HallucinationGhost.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"
#include "CollisionShape.h"

#include <cfloat>

AMonsterSpawner::AMonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

const TCHAR* AMonsterSpawner::GetFailReasonText(ESpawnFailReason Reason)
{
    switch (Reason)
    {
    case ESpawnFailReason::None: return TEXT("None");
    case ESpawnFailReason::NoClass: return TEXT("NoClass");
    case ESpawnFailReason::NoSurfaceSample: return TEXT("NoSurfaceSample");
    case ESpawnFailReason::OutsideFrontBand: return TEXT("OutsideFrontBand");
    case ESpawnFailReason::SameTypeDistance: return TEXT("SameTypeDistance");
    case ESpawnFailReason::InsideRock: return TEXT("InsideRock");
    case ESpawnFailReason::HeightTraceFailed: return TEXT("HeightTraceFailed");
    case ESpawnFailReason::HeightOutOfRange: return TEXT("HeightOutOfRange");
    case ESpawnFailReason::ClearanceFailed: return TEXT("ClearanceFailed");
    case ESpawnFailReason::TerritoryFailed: return TEXT("TerritoryFailed");
    case ESpawnFailReason::FacingTraceFailed: return TEXT("FacingTraceFailed");
    case ESpawnFailReason::FacingNormalInvalid: return TEXT("FacingNormalInvalid");
    case ESpawnFailReason::SpawnActorFailed: return TEXT("SpawnActorFailed");
    case ESpawnFailReason::OverlapBlocked: return TEXT("OverlapBlocked");
    default: return TEXT("Unknown");
    }
}

void AMonsterSpawner::LogFailStats(const TCHAR* Label, const FSpawnFailStats& Stats, int32 Spawned, int32 Target) const
{
    FString Line = FString::Printf(TEXT("%s %d/%d"), Label, Spawned, Target);

    for (int32 i = 1; i < (int32)ESpawnFailReason::Count; ++i)
    {
        const ESpawnFailReason Reason = (ESpawnFailReason)i;
        const int32 Count = Stats.Get(Reason);
        if (Count > 0)
        {
            Line += FString::Printf(TEXT(" | %s=%d"), GetFailReasonText(Reason), Count);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("%s"), *Line);
}

void AMonsterSpawner::BeginPlay()
{
    Super::BeginPlay();

    LastObservedPlacementSeed = PlacementSeed;

    ResolveMountain();

    if (TargetMountain && bSpawnOnlyAfterMountainGenerated)
    {
        TargetMountain->OnMountainGenerated.RemoveDynamic(this, &AMonsterSpawner::HandleMountainGenerated);
        TargetMountain->OnMountainGenerated.AddDynamic(this, &AMonsterSpawner::HandleMountainGenerated);

        if (bAutoSpawnOnBeginPlay && TargetMountain->HasGeneratedMesh())
        {
            SpawnMonsters();
        }
        else if (bAutoSpawnOnBeginPlay && GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                DeferredInitialSpawnTimer,
                this,
                &AMonsterSpawner::TryInitialSpawnFallback,
                0.25f,
                false
            );
        }
    }
    else if (bAutoSpawnOnBeginPlay)
    {
        SpawnMonsters();
    }
}

void AMonsterSpawner::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bAutoRespawnWhenPlacementSeedChanges)
    {
        LastObservedPlacementSeed = PlacementSeed;
        return;
    }

    if (LastObservedPlacementSeed == PlacementSeed)
    {
        return;
    }

    const int32 OldSeed = LastObservedPlacementSeed;
    LastObservedPlacementSeed = PlacementSeed;

    if (bVerboseLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] PlacementSeed changed %d -> %d. Clear and respawn monsters."), OldSeed, PlacementSeed);
    }

    RespawnMonsters();
}

void AMonsterSpawner::TryInitialSpawnFallback()
{
    if (!TargetMountain)
    {
        ResolveMountain();
    }

    if (TargetMountain && TargetMountain->HasGeneratedMesh() && SpawnedMonsters.Num() == 0)
    {
        SpawnMonsters();
    }
}

void AMonsterSpawner::HandleMountainGenerated(AActor* Generator)
{
    if (!Generator || Generator != TargetMountain.Get())
    {
        return;
    }

    if (!bRespawnWhenMountainRegenerates && SpawnedMonsters.Num() > 0)
    {
        return;
    }

    SpawnMonsters();
}

bool AMonsterSpawner::ResolveMountain()
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

bool AMonsterSpawner::GetMountainBounds(FBox& OutBounds) const
{
    OutBounds = FBox(EForceInit::ForceInit);

    if (!TargetMountain || !TargetMountain->HasGeneratedMesh())
    {
        return false;
    }

    OutBounds = TargetMountain->GetGeneratedWorldBounds();
    return OutBounds.IsValid != 0;
}

bool AMonsterSpawner::GetFrontBandBounds(FBox& OutBounds) const
{
    OutBounds = FBox(EForceInit::ForceInit);

    FBox MountainBounds;
    if (!GetMountainBounds(MountainBounds) || !TargetMountain)
    {
        return false;
    }

    const float AllowedDepth =
        (FrontSpawnDepthOverrideCm > 0.0f)
        ? FrontSpawnDepthOverrideCm
        : TargetMountain->GetSuggestedFrontSpawnDepthCm();

    const float FrontX = TargetMountain->GetFrontSurfaceWorldX();

    const float MinX = FrontX - 50.0f;
    const float MaxX = FrontX + AllowedDepth;

    const float YMinRaw = MountainBounds.Min.Y + SideSpawnMarginCm;
    const float YMaxRaw = MountainBounds.Max.Y - SideSpawnMarginCm;
    const float YMin = (YMinRaw < YMaxRaw) ? YMinRaw : MountainBounds.Min.Y;
    const float YMax = (YMinRaw < YMaxRaw) ? YMaxRaw : MountainBounds.Max.Y;

    const FVector Min(
        MinX,
        YMin,
        MountainBounds.Min.Z + 80.0f
    );

    const FVector Max(
        MaxX,
        YMax,
        MountainBounds.Max.Z + 2500.0f
    );

    OutBounds = FBox(Min, Max);
    return OutBounds.IsValid != 0;
}

float AMonsterSpawner::GetMinDistanceForKind(EMonsterSpawnKind Kind) const
{
    switch (Kind)
    {
    case EMonsterSpawnKind::WallCrawler:
        return WallCrawlerMinDistance;
    case EMonsterSpawnKind::FlyingPlatform:
        return FlyingPlatformMinDistance;
    case EMonsterSpawnKind::FlyingAttacker:
        return FlyingAttackerMinDistance;
    default:
        return 0.0f;
    }
}

int32 AMonsterSpawner::MakePlacementStreamSeed(EMonsterSpawnKind Kind) const
{
    uint32 Hash = GetTypeHash(PlacementSeed);

    if (TargetMountain)
    {
        Hash = HashCombine(Hash, GetTypeHash(TargetMountain->Settings.Seed));
        Hash = HashCombine(Hash, GetTypeHash((uint8)TargetMountain->Settings.Difficulty));
    }

    Hash = HashCombine(Hash, GetTypeHash((uint8)Kind));

    return static_cast<int32>(Hash & 0x7fffffff);
}

bool AMonsterSpawner::IsFarEnoughFromSameType(const FVector& Location, EMonsterSpawnKind Kind) const
{
    const float RequiredDistance = GetMinDistanceForKind(Kind);

    if (RequiredDistance <= 0.0f)
    {
        return true;
    }

    for (AMonsterBase* Monster : SpawnedMonsters)
    {
        if (!IsValid(Monster))
        {
            continue;
        }

        bool bSameType = false;

        switch (Kind)
        {
        case EMonsterSpawnKind::WallCrawler:
            bSameType = Monster->IsA(AWallCrawler::StaticClass());
            break;
        case EMonsterSpawnKind::FlyingPlatform:
            bSameType = Monster->IsA(AFlyingPlatform::StaticClass());
            break;
        case EMonsterSpawnKind::FlyingAttacker:
            bSameType = Monster->IsA(AFlyingAttacker::StaticClass());
            break;
        default:
            break;
        }

        if (!bSameType)
        {
            continue;
        }

        if (FVector::Dist(Monster->GetActorLocation(), Location) < RequiredDistance)
        {
            return false;
        }
    }

    return true;
}

float AMonsterSpawner::GetNearestSameTypeDistance(const FVector& Location, EMonsterSpawnKind Kind) const
{
    float NearestDist = BIG_NUMBER;

    for (AMonsterBase* Monster : SpawnedMonsters)
    {
        if (!IsValid(Monster))
        {
            continue;
        }

        bool bSameType = false;

        switch (Kind)
        {
        case EMonsterSpawnKind::WallCrawler:
            bSameType = Monster->IsA(AWallCrawler::StaticClass());
            break;
        case EMonsterSpawnKind::FlyingPlatform:
            bSameType = Monster->IsA(AFlyingPlatform::StaticClass());
            break;
        case EMonsterSpawnKind::FlyingAttacker:
            bSameType = Monster->IsA(AFlyingAttacker::StaticClass());
            break;
        default:
            break;
        }

        if (!bSameType)
        {
            continue;
        }

        const float Dist = FVector::Dist(Monster->GetActorLocation(), Location);
        NearestDist = FMath::Min(NearestDist, Dist);
    }

    return NearestDist;
}

bool AMonsterSpawner::IsInsideRock(const FVector& Location, float CheckDistance) const
{
    if (!GetWorld())
    {
        return true;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterInsideRock), false);
    Params.AddIgnoredActor(this);

    static const FVector Directions[] =
    {
        FVector(1, 0, 0),
        FVector(-1, 0, 0),
        FVector(0, 1, 0),
        FVector(0, -1, 0),
        FVector(0, 0, 1),
        FVector(0, 0, -1)
    };

    int32 BlockedCount = 0;

    for (const FVector& Dir : Directions)
    {
        FHitResult Hit;
        const FVector End = Location + Dir * CheckDistance;

        if (GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECC_WorldStatic, Params))
        {
            if (Hit.Distance < CheckDistance * 0.75f)
            {
                BlockedCount++;
            }
        }
    }

    return BlockedCount >= 4;
}

bool AMonsterSpawner::HasLocalClearance(const FVector& Location, float Radius, float HalfHeight) const
{
    if (!GetWorld())
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterClearance), false);
    Params.AddIgnoredActor(this);

    const TArray<FVector> Directions =
    {
        FVector::ForwardVector,
        -FVector::ForwardVector,
        FVector::RightVector,
        -FVector::RightVector,
        FVector::UpVector,
        -FVector::UpVector,
        FVector(1, 1, 0).GetSafeNormal(),
        FVector(-1, 1, 0).GetSafeNormal(),
        FVector(1, -1, 0).GetSafeNormal(),
        FVector(-1, -1, 0).GetSafeNormal()
    };

    int32 TightHits = 0;

    for (const FVector& Dir : Directions)
    {
        const float Dist = (FMath::Abs(Dir.Z) > 0.5f) ? HalfHeight : Radius;

        FHitResult Hit;
        const FVector End = Location + Dir * Dist;

        if (GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECC_WorldStatic, Params))
        {
            if (Hit.Distance < Dist * 0.7f)
            {
                TightHits++;
            }
        }
    }

    return TightHits <= 2;
}

bool AMonsterSpawner::TraceHeightAboveGround(const FVector& Location, float& OutHeight) const
{
    OutHeight = 0.0f;

    if (!GetWorld())
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterGroundHeight), false);
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    const FVector Start = Location;
    const FVector End = Location - FVector(0, 0, 10000.0f);

    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        return false;
    }

    OutHeight = Hit.Distance;
    return true;
}

bool AMonsterSpawner::IsSphereOverlappingWorldStatic(const FVector& Location, float Radius) const
{
    if (!GetWorld())
    {
        return true;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterOverlapCheck), false);
    Params.AddIgnoredActor(this);

    return GetWorld()->OverlapBlockingTestByChannel(
        Location,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius),
        Params
    );
}

bool AMonsterSpawner::PushOutFromWall(FVector& InOutLocation, const FVector& WallNormal, float Radius) const
{
    if (!GetWorld())
    {
        return false;
    }

    FVector TestLocation = InOutLocation;

    for (int32 Step = 0; Step < PlatformMaxPushOutSteps; ++Step)
    {
        if (!IsSphereOverlappingWorldStatic(TestLocation, Radius))
        {
            InOutLocation = TestLocation;
            return true;
        }

        TestLocation += WallNormal * PlatformPushOutStep;
    }

    if (!IsSphereOverlappingWorldStatic(TestLocation, Radius))
    {
        InOutLocation = TestLocation;
        return true;
    }

    return false;
}

bool AMonsterSpawner::TraceMountainOnly(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
    if (!GetWorld() || !TargetMountain)
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MountainOnlyTrace), false);
    Params.AddIgnoredActor(this);

    if (!GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, Params))
    {
        return false;
    }

    return OutHit.GetActor() == TargetMountain;
}

bool AMonsterSpawner::IsPointWithinFrontSpawnBand(const FVector& WorldPoint) const
{
    if (!TargetMountain)
    {
        return false;
    }

    // Mountain density makes the solid cliff extend toward -X.
    // Therefore the actual front/playable air side is +X from GetFrontSurfaceWorldX().
    const float SignedFrontDepth = WorldPoint.X - TargetMountain->GetFrontSurfaceWorldX();
    if (SignedFrontDepth < -50.0f)
    {
        return false;
    }

    const float AllowedDepth =
        (FrontSpawnDepthOverrideCm > 0.0f)
        ? FrontSpawnDepthOverrideCm
        : TargetMountain->GetSuggestedFrontSpawnDepthCm();

    if (SignedFrontDepth > AllowedDepth)
    {
        return false;
    }

    FBox Bounds;
    if (!GetMountainBounds(Bounds))
    {
        return false;
    }

    const float YMinRaw = Bounds.Min.Y + SideSpawnMarginCm;
    const float YMaxRaw = Bounds.Max.Y - SideSpawnMarginCm;
    const float YMin = (YMinRaw < YMaxRaw) ? YMinRaw : Bounds.Min.Y;
    const float YMax = (YMinRaw < YMaxRaw) ? YMaxRaw : Bounds.Max.Y;

    return WorldPoint.Y >= YMin && WorldPoint.Y <= YMax;
}

bool AMonsterSpawner::IsFrontFacingSurface(const FHitResult& Hit) const
{
    if (!TargetMountain || Hit.GetActor() != TargetMountain)
    {
        return false;
    }

    if (!IsPointWithinFrontSpawnBand(Hit.ImpactPoint))
    {
        return false;
    }

    const FVector FrontDir = FVector::ForwardVector; // +X is the actual cliff front side.
    return FVector::DotProduct(Hit.ImpactNormal.GetSafeNormal(), FrontDir) >= FrontFacingNormalDotMin;
}

bool AMonsterSpawner::TraceFrontCliffFromAir(const FVector& AirLocation, float TraceDistance, FHitResult& OutHit) const
{
    if (!GetWorld() || !TargetMountain)
    {
        return false;
    }

    const float SafeDistance = FMath::Max(100.0f, TraceDistance);
    const FVector FrontDir = FVector::ForwardVector;

    // Start from the +X air side and trace backward into the cliff.
    const FVector Start = AirLocation + FrontDir * 20.0f;
    const FVector End = AirLocation - FrontDir * SafeDistance;

    FHitResult Hit;
    if (!TraceMountainOnly(Start, End, Hit))
    {
        return false;
    }

    if (!IsFrontFacingSurface(Hit))
    {
        return false;
    }

    OutHit = Hit;
    return true;
}

bool AMonsterSpawner::SampleMountainSurface(FRandomStream& Stream, FHitResult& OutHit, float MinAbsNormalZ, float MaxAbsNormalZ, ESpawnFailReason* OutFailReason) const
{
    if (OutFailReason)
    {
        *OutFailReason = ESpawnFailReason::NoSurfaceSample;
    }

    FBox Bounds;
    if (!GetMountainBounds(Bounds) || !TargetMountain)
    {
        return false;
    }

    const float FrontX = TargetMountain->GetFrontSurfaceWorldX();
    const float TraceBackDistance = FMath::Max(
        Bounds.GetSize().X + BoundsPadding + 1000.0f,
        FrontSpawnDepthOverrideCm + 2000.0f
    );

    const float YMinRaw = Bounds.Min.Y + SideSpawnMarginCm;
    const float YMaxRaw = Bounds.Max.Y - SideSpawnMarginCm;
    const float YMin = (YMinRaw < YMaxRaw) ? YMinRaw : Bounds.Min.Y;
    const float YMax = (YMinRaw < YMaxRaw) ? YMaxRaw : Bounds.Max.Y;
    const float ZMin = Bounds.Min.Z + 80.0f;
    const float ZMax = Bounds.Max.Z + 500.0f;

    bool bSawFrontBandReject = false;

    // Do not shoot random rays through the whole mountain.
    // Shoot only from +X front air toward -X, so the first hit is the visible front cliff.
    for (int32 Attempt = 0; Attempt < 128; ++Attempt)
    {
        const FVector Start(
            FrontX + FMath::Max(800.0f, FrontSpawnDepthOverrideCm),
            Stream.FRandRange(YMin, YMax),
            Stream.FRandRange(ZMin, ZMax)
        );

        const FVector End = Start - FVector::ForwardVector * TraceBackDistance;

        FHitResult Hit;
        if (!TraceMountainOnly(Start, End, Hit))
        {
            continue;
        }

        if (!IsPointWithinFrontSpawnBand(Hit.ImpactPoint))
        {
            bSawFrontBandReject = true;
            continue;
        }

        if (!IsFrontFacingSurface(Hit))
        {
            continue;
        }

        const float AbsNormalZ = FMath::Abs(Hit.ImpactNormal.Z);
        if (AbsNormalZ < MinAbsNormalZ || AbsNormalZ > MaxAbsNormalZ)
        {
            continue;
        }

        OutHit = Hit;
        if (OutFailReason)
        {
            *OutFailReason = ESpawnFailReason::None;
        }
        return true;
    }

    if (OutFailReason && bSawFrontBandReject)
    {
        *OutFailReason = ESpawnFailReason::OutsideFrontBand;
    }

    return false;
}

bool AMonsterSpawner::SampleFrontBandAirLocation(FRandomStream& Stream, FVector& OutLocation, float HorizontalRadius, float MinZOffset, float MaxZOffset, EMonsterSpawnKind Kind) const
{
    OutLocation = FVector::ZeroVector;

    FBox FrontBounds;
    if (!GetFrontBandBounds(FrontBounds))
    {
        return false;
    }

    bool bFoundAny = false;
    float BestScore = -FLT_MAX;
    FVector BestLocation = FVector::ZeroVector;

    const int32 CandidateTries = FMath::Max(8, FrontBandCandidateTries);

    for (int32 Attempt = 0; Attempt < CandidateTries; ++Attempt)
    {
        const FVector Candidate(
            Stream.FRandRange(FrontBounds.Min.X, FrontBounds.Max.X),
            Stream.FRandRange(FrontBounds.Min.Y, FrontBounds.Max.Y),
            Stream.FRandRange(FrontBounds.Min.Z, FrontBounds.Max.Z)
        );

        if (!IsPointWithinFrontSpawnBand(Candidate))
        {
            continue;
        }

        if (IsInsideRock(Candidate, 60.0f))
        {
            continue;
        }

        float HeightAboveGround = 0.0f;
        if (!TraceHeightAboveGround(Candidate, HeightAboveGround))
        {
            continue;
        }

        if (HeightAboveGround < MinZOffset || HeightAboveGround > MaxZOffset)
        {
            continue;
        }

        if (!HasLocalClearance(Candidate, HorizontalRadius, FMath::Max(120.0f, HorizontalRadius * 0.5f)))
        {
            continue;
        }

        if (IsSphereOverlappingWorldStatic(Candidate, FMath::Max(60.0f, HorizontalRadius * 0.45f)))
        {
            continue;
        }

        if (!IsFarEnoughFromSameType(Candidate, Kind))
        {
            continue;
        }

        float NearestSameTypeDist = GetNearestSameTypeDistance(Candidate, Kind);
        if (NearestSameTypeDist == BIG_NUMBER)
        {
            NearestSameTypeDist = 100000.0f;
        }

        const float CenterY = (FrontBounds.Min.Y + FrontBounds.Max.Y) * 0.5f;
        const float HalfY = FMath::Max(1.0f, (FrontBounds.Max.Y - FrontBounds.Min.Y) * 0.5f);
        const float YCenteredness = 1.0f - FMath::Abs(Candidate.Y - CenterY) / HalfY;

        // 같은 종류와의 분산이 핵심. Y 보정은 약하게만 준다.
        const float Score = NearestSameTypeDist + (YCenteredness * 50.0f);

        if (!bFoundAny || Score > BestScore)
        {
            bFoundAny = true;
            BestScore = Score;
            BestLocation = Candidate;
        }
    }

    if (!bFoundAny)
    {
        return false;
    }

    OutLocation = BestLocation;
    return true;
}

bool AMonsterSpawner::FindWallCrawlerSpawn(FRandomStream& Stream, FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const
{
    OutResult = FSpawnProbeResult();
    OutFailReason = ESpawnFailReason::None;

    if (!WallCrawlerClass)
    {
        OutFailReason = ESpawnFailReason::NoClass;
        return false;
    }

    // 단순화된 규칙:
    // 1) 모든 몬스터는 먼저 절벽 앞면 표면을 찾는다.
    // 2) WallCrawler는 그 표면에서 살짝 띄우고, 벽을 바라보게 회전한다.
    FHitResult SurfaceHit;
    if (!SampleMountainSurface(Stream, SurfaceHit, WallMinAbsNormalZ, WallMaxAbsNormalZ, &OutFailReason))
    {
        return false;
    }

    const FVector SurfaceNormal = SurfaceHit.ImpactNormal.GetSafeNormal();
    const FVector CandidateLocation =
        SurfaceHit.ImpactPoint + SurfaceNormal * FMath::Max(0.0f, WallCrawlerSpawnOffset);

    if (!IsPointWithinFrontSpawnBand(CandidateLocation))
    {
        OutFailReason = ESpawnFailReason::OutsideFrontBand;
        return false;
    }

    if (!IsFarEnoughFromSameType(CandidateLocation, EMonsterSpawnKind::WallCrawler))
    {
        OutFailReason = ESpawnFailReason::SameTypeDistance;
        return false;
    }

    OutResult.bSuccess = true;
    OutResult.Location = CandidateLocation;
    OutResult.Rotation = (-SurfaceNormal).Rotation() + WallCrawlerVisualRotationOffset;
    OutResult.SurfacePoint = SurfaceHit.ImpactPoint;
    OutResult.SurfaceNormal = SurfaceNormal;
    return true;
}

bool AMonsterSpawner::FindFlyingPlatformSpawn(FRandomStream& Stream, FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const
{
    OutResult = FSpawnProbeResult();
    OutFailReason = ESpawnFailReason::None;

    if (!FlyingPlatformClass)
    {
        OutFailReason = ESpawnFailReason::NoClass;
        return false;
    }

    // 단순화된 규칙:
    // 먼저 절벽 앞면 표면을 찾고, FlyingPlatform은 벽 노말 방향으로 멀리 띄운다.
    FHitResult SurfaceHit;
    if (!SampleMountainSurface(Stream, SurfaceHit, 0.0f, 1.0f, &OutFailReason))
    {
        return false;
    }

    const FVector SurfaceNormal = SurfaceHit.ImpactNormal.GetSafeNormal();
    const float MinDistance = FMath::Max(0.0f, FMath::Min(PlatformMinWallDistance, PlatformMaxWallDistance));
    const float MaxDistance = FMath::Max(MinDistance, FMath::Max(PlatformMinWallDistance, PlatformMaxWallDistance));
    const float WallOffset = Stream.FRandRange(MinDistance, MaxDistance);

    FVector CandidateLocation = SurfaceHit.ImpactPoint + SurfaceNormal * WallOffset;

    // 플랫폼이 아주 얕게 벽과 겹치는 경우만 바깥쪽으로 살짝 밀어낸다.
    PushOutFromWall(CandidateLocation, SurfaceNormal, PlatformOverlapCheckRadius);

    if (!IsPointWithinFrontSpawnBand(CandidateLocation))
    {
        OutFailReason = ESpawnFailReason::OutsideFrontBand;
        return false;
    }

    if (!IsFarEnoughFromSameType(CandidateLocation, EMonsterSpawnKind::FlyingPlatform))
    {
        OutFailReason = ESpawnFailReason::SameTypeDistance;
        return false;
    }

    OutResult.bSuccess = true;
    OutResult.Location = CandidateLocation;
    OutResult.Rotation = FRotator::ZeroRotator;
    OutResult.SurfacePoint = SurfaceHit.ImpactPoint;
    OutResult.SurfaceNormal = SurfaceNormal;
    return true;
}

bool AMonsterSpawner::EvaluateFlyingAttackerTerritory(const FVector& CandidateLocation, float TerritoryRadius) const
{
    if (!GetWorld())
    {
        return false;
    }

    const TArray<FVector> OctantDirections =
    {
        FVector(1, 1, 1).GetSafeNormal(),
        FVector(-1, 1, 1).GetSafeNormal(),
        FVector(1, -1, 1).GetSafeNormal(),
        FVector(-1, -1, 1).GetSafeNormal(),
        FVector(1, 1, -1).GetSafeNormal(),
        FVector(-1, 1, -1).GetSafeNormal(),
        FVector(1, -1, -1).GetSafeNormal(),
        FVector(-1, -1, -1).GetSafeNormal()
    };

    FCollisionQueryParams Params(SCENE_QUERY_STAT(FlyingAttackerTerritory), false);
    Params.AddIgnoredActor(this);

    const float RequiredMinDistance = TerritoryRadius * AttackerRequiredOctantDistanceRatio;
    const float RequiredAverageDistance = TerritoryRadius * AttackerRequiredAverageDistanceRatio;

    int32 OpenOctants = 0;
    float TotalDistance = 0.0f;

    for (const FVector& Dir : OctantDirections)
    {
        FHitResult Hit;
        const FVector End = CandidateLocation + Dir * TerritoryRadius;

        float MaxDistance = TerritoryRadius;
        if (GetWorld()->LineTraceSingleByChannel(Hit, CandidateLocation, End, ECC_WorldStatic, Params))
        {
            MaxDistance = FMath::Min(Hit.Distance, TerritoryRadius);
        }

        TotalDistance += MaxDistance;

        if (MaxDistance >= RequiredMinDistance)
        {
            OpenOctants++;
        }
    }

    const float AverageDistance = TotalDistance / 8.0f;

    return OpenOctants >= AttackerRequiredOpenOctants
        && AverageDistance >= RequiredAverageDistance;
}

bool AMonsterSpawner::FindFlyingAttackerSpawn(FRandomStream& Stream, FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const
{
    OutResult = FSpawnProbeResult();
    OutFailReason = ESpawnFailReason::None;

    if (!FlyingAttackerClass)
    {
        OutFailReason = ESpawnFailReason::NoClass;
        return false;
    }

    // 단순화된 규칙:
    // FlyingAttacker도 절벽 앞면 표면을 기준으로 잡고, 벽에서 약 1m 이상만 떨어뜨린다.
    FHitResult SurfaceHit;
    if (!SampleMountainSurface(Stream, SurfaceHit, 0.0f, 1.0f, &OutFailReason))
    {
        return false;
    }

    const FVector SurfaceNormal = SurfaceHit.ImpactNormal.GetSafeNormal();
    const float MinDistance = FMath::Max(0.0f, FMath::Min(AttackerMinWallDistance, AttackerMaxWallDistance));
    const float MaxDistance = FMath::Max(MinDistance, FMath::Max(AttackerMinWallDistance, AttackerMaxWallDistance));
    const float WallOffset = Stream.FRandRange(MinDistance, MaxDistance);

    const FVector CandidateLocation = SurfaceHit.ImpactPoint + SurfaceNormal * WallOffset;

    if (!IsPointWithinFrontSpawnBand(CandidateLocation))
    {
        OutFailReason = ESpawnFailReason::OutsideFrontBand;
        return false;
    }

    if (!IsFarEnoughFromSameType(CandidateLocation, EMonsterSpawnKind::FlyingAttacker))
    {
        OutFailReason = ESpawnFailReason::SameTypeDistance;
        return false;
    }

    OutResult.bSuccess = true;
    OutResult.Location = CandidateLocation;
    OutResult.Rotation = FRotator::ZeroRotator;
    OutResult.SurfacePoint = SurfaceHit.ImpactPoint;
    OutResult.SurfaceNormal = SurfaceNormal;
    return true;
}



static bool MG_IsFarEnoughFromPreparedResults(const TArray<FSpawnProbeResult>& PreparedResults, const FVector& Location, float RequiredDistance)
{
    if (RequiredDistance <= 0.0f)
    {
        return true;
    }

    for (const FSpawnProbeResult& Prepared : PreparedResults)
    {
        if (!Prepared.bSuccess)
        {
            continue;
        }

        if (FVector::Dist(Prepared.Location, Location) < RequiredDistance)
        {
            return false;
        }
    }

    return true;
}

bool AMonsterSpawner::PrepareSpawnResults(
    TArray<FSpawnProbeResult>& OutWallResults,
    TArray<FSpawnProbeResult>& OutPlatformResults,
    TArray<FSpawnProbeResult>& OutAttackerResults,
    FSpawnFailStats& OutWallStats,
    FSpawnFailStats& OutPlatformStats,
    FSpawnFailStats& OutAttackerStats) const
{
    OutWallResults.Reset();
    OutPlatformResults.Reset();
    OutAttackerResults.Reset();

    OutWallStats = FSpawnFailStats();
    OutPlatformStats = FSpawnFailStats();
    OutAttackerStats = FSpawnFailStats();

    FRandomStream WallStream(MakePlacementStreamSeed(EMonsterSpawnKind::WallCrawler));
    FRandomStream PlatformStream(MakePlacementStreamSeed(EMonsterSpawnKind::FlyingPlatform));
    FRandomStream AttackerStream(MakePlacementStreamSeed(EMonsterSpawnKind::FlyingAttacker));

    if (WallCrawlerCount > 0 && !WallCrawlerClass)
    {
        OutWallStats.Add(ESpawnFailReason::NoClass);
    }
    else if (WallCrawlerClass)
    {
        for (int32 Attempt = 0; Attempt < MaxAttemptsPerWallCrawler && OutWallResults.Num() < WallCrawlerCount; ++Attempt)
        {
            FSpawnProbeResult Result;
            ESpawnFailReason FailReason = ESpawnFailReason::None;

            if (!FindWallCrawlerSpawn(WallStream, Result, FailReason))
            {
                OutWallStats.Add(FailReason);
                continue;
            }

            if (!MG_IsFarEnoughFromPreparedResults(OutWallResults, Result.Location, WallCrawlerMinDistance))
            {
                OutWallStats.Add(ESpawnFailReason::SameTypeDistance);
                continue;
            }

            Result.bSuccess = true;
            OutWallResults.Add(Result);
        }
    }

    if (FlyingPlatformCount > 0 && !FlyingPlatformClass)
    {
        OutPlatformStats.Add(ESpawnFailReason::NoClass);
    }
    else if (FlyingPlatformClass)
    {
        for (int32 Attempt = 0; Attempt < MaxAttemptsPerFlyingPlatform && OutPlatformResults.Num() < FlyingPlatformCount; ++Attempt)
        {
            FSpawnProbeResult Result;
            ESpawnFailReason FailReason = ESpawnFailReason::None;

            if (!FindFlyingPlatformSpawn(PlatformStream, Result, FailReason))
            {
                OutPlatformStats.Add(FailReason);
                continue;
            }

            if (!MG_IsFarEnoughFromPreparedResults(OutPlatformResults, Result.Location, FlyingPlatformMinDistance))
            {
                OutPlatformStats.Add(ESpawnFailReason::SameTypeDistance);
                continue;
            }

            Result.bSuccess = true;
            OutPlatformResults.Add(Result);
        }
    }

    if (FlyingAttackerCount > 0 && !FlyingAttackerClass)
    {
        OutAttackerStats.Add(ESpawnFailReason::NoClass);
    }
    else if (FlyingAttackerClass)
    {
        for (int32 Attempt = 0; Attempt < MaxAttemptsPerFlyingAttacker && OutAttackerResults.Num() < FlyingAttackerCount; ++Attempt)
        {
            FSpawnProbeResult Result;
            ESpawnFailReason FailReason = ESpawnFailReason::None;

            if (!FindFlyingAttackerSpawn(AttackerStream, Result, FailReason))
            {
                OutAttackerStats.Add(FailReason);
                continue;
            }

            if (!MG_IsFarEnoughFromPreparedResults(OutAttackerResults, Result.Location, FlyingAttackerMinDistance))
            {
                OutAttackerStats.Add(ESpawnFailReason::SameTypeDistance);
                continue;
            }

            Result.bSuccess = true;
            OutAttackerResults.Add(Result);
        }
    }

    return (OutWallResults.Num() + OutPlatformResults.Num() + OutAttackerResults.Num()) > 0;
}

bool AMonsterSpawner::SpawnPreparedMonsters(
    const TArray<FSpawnProbeResult>& WallResults,
    const TArray<FSpawnProbeResult>& PlatformResults,
    const TArray<FSpawnProbeResult>& AttackerResults,
    TArray<AMonsterBase*>& OutSpawnedMonsters,
    FSpawnFailStats& InOutWallStats,
    FSpawnFailStats& InOutPlatformStats,
    FSpawnFailStats& InOutAttackerStats) const
{
    OutSpawnedMonsters.Reset();

    if (!GetWorld())
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (const FSpawnProbeResult& Result : WallResults)
    {
        AWallCrawler* Spawned = GetWorld()->SpawnActor<AWallCrawler>(
            WallCrawlerClass,
            Result.Location,
            Result.Rotation,
            SpawnParams);

        if (!Spawned)
        {
            InOutWallStats.Add(ESpawnFailReason::SpawnActorFailed);
            return false;
        }

        OutSpawnedMonsters.Add(Spawned);

        if (bShowDebugPoints)
        {
            DrawDebugSphere(GetWorld(), Result.Location, 35.0f, 12, FColor::Green, false, 12.0f, 0, 2.0f);
            DrawDebugLine(GetWorld(), Result.Location, Result.SurfacePoint, FColor::Green, false, 12.0f, 0, 2.0f);
        }
    }

    for (const FSpawnProbeResult& Result : PlatformResults)
    {
        AFlyingPlatform* Spawned = GetWorld()->SpawnActor<AFlyingPlatform>(
            FlyingPlatformClass,
            Result.Location,
            Result.Rotation,
            SpawnParams);

        if (!Spawned)
        {
            InOutPlatformStats.Add(ESpawnFailReason::SpawnActorFailed);
            return false;
        }

        OutSpawnedMonsters.Add(Spawned);

        if (bShowDebugPoints)
        {
            DrawDebugSphere(GetWorld(), Result.Location, 35.0f, 12, FColor::Blue, false, 12.0f, 0, 2.0f);
            DrawDebugLine(GetWorld(), Result.SurfacePoint, Result.Location, FColor::Blue, false, 12.0f, 0, 2.0f);
        }
    }

    for (const FSpawnProbeResult& Result : AttackerResults)
    {
        AFlyingAttacker* Spawned = GetWorld()->SpawnActor<AFlyingAttacker>(
            FlyingAttackerClass,
            Result.Location,
            Result.Rotation,
            SpawnParams);

        if (!Spawned)
        {
            InOutAttackerStats.Add(ESpawnFailReason::SpawnActorFailed);
            return false;
        }

        OutSpawnedMonsters.Add(Spawned);

        if (bShowDebugPoints)
        {
            DrawDebugSphere(GetWorld(), Result.Location, 40.0f, 12, FColor::Red, false, 12.0f, 0, 2.0f);
            DrawDebugLine(GetWorld(), Result.SurfacePoint, Result.Location, FColor::Red, false, 12.0f, 0, 2.0f);
        }
    }

    return true;
}

void AMonsterSpawner::SpawnMonsters()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterSpawner: invalid world"));
        return;
    }

    if (!ResolveMountain())
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterSpawner: no mountain actor found"));
        return;
    }

    if (!TargetMountain->HasGeneratedMesh())
    {
        UE_LOG(LogTemp, Warning, TEXT("MonsterSpawner: mountain mesh is not ready"));
        return;
    }

    LastObservedPlacementSeed = PlacementSeed;

    TArray<AMonsterBase*> PreviousMonsters;

    if (bClearExistingMonstersBeforeSpawn)
    {
        // ItemSpawner처럼 새 배치를 만들기 전에 기존 몬스터를 먼저 제거한다.
        ClearSpawnedMonsters();
    }
    else
    {
        // 실패 시 기존 몬스터를 유지하고 싶을 때만 예전 방식으로 보관한다.
        PreviousMonsters = MoveTemp(SpawnedMonsters);
        SpawnedMonsters.Reset();
    }

    TArray<FSpawnProbeResult> WallResults;
    TArray<FSpawnProbeResult> PlatformResults;
    TArray<FSpawnProbeResult> AttackerResults;

    FSpawnFailStats WallStats;
    FSpawnFailStats PlatformStats;
    FSpawnFailStats AttackerStats;

    const bool bPreparedAny = PrepareSpawnResults(
        WallResults,
        PlatformResults,
        AttackerResults,
        WallStats,
        PlatformStats,
        AttackerStats);

    LogFailStats(TEXT("[MonsterSpawner][Prepared WallCrawler]"), WallStats, WallResults.Num(), WallCrawlerCount);
    LogFailStats(TEXT("[MonsterSpawner][Prepared FlyingPlatform]"), PlatformStats, PlatformResults.Num(), FlyingPlatformCount);
    LogFailStats(TEXT("[MonsterSpawner][Prepared FlyingAttacker]"), AttackerStats, AttackerResults.Num(), FlyingAttackerCount);

    if (!bPreparedAny)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] No validated spawn candidates. No SpawnActor executed."));

        if (!bClearExistingMonstersBeforeSpawn)
        {
            SpawnedMonsters = MoveTemp(PreviousMonsters);
        }

        return;
    }

    TArray<AMonsterBase*> NewSpawnedMonsters;
    const bool bSpawnSucceeded = SpawnPreparedMonsters(
        WallResults,
        PlatformResults,
        AttackerResults,
        NewSpawnedMonsters,
        WallStats,
        PlatformStats,
        AttackerStats);

    if (!bSpawnSucceeded)
    {
        for (AMonsterBase* Monster : NewSpawnedMonsters)
        {
            if (IsValid(Monster))
            {
                Monster->Destroy();
            }
        }

        UE_LOG(LogTemp, Error, TEXT("[MonsterSpawner] SpawnActor failed after preparation. New spawned monsters were removed."));

        if (!bClearExistingMonstersBeforeSpawn)
        {
            SpawnedMonsters = MoveTemp(PreviousMonsters);
        }

        return;
    }

    if (!bClearExistingMonstersBeforeSpawn)
    {
        for (AMonsterBase* Monster : PreviousMonsters)
        {
            if (IsValid(Monster))
            {
                Monster->Destroy();
            }
        }
    }

    SpawnedMonsters = MoveTemp(NewSpawnedMonsters);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[MonsterSpawner] Final SpawnActor Result | PlacementSeed=%d | Wall %d/%d | Platform %d/%d | Attacker %d/%d | Total %d"),
        PlacementSeed,
        WallResults.Num(), WallCrawlerCount,
        PlatformResults.Num(), FlyingPlatformCount,
        AttackerResults.Num(), FlyingAttackerCount,
        SpawnedMonsters.Num()
    );
}

void AMonsterSpawner::RespawnMonsters()
{
    ClearSpawnedMonsters();
    SpawnMonsters();
}

void AMonsterSpawner::SetPlacementSeedAndRespawn(int32 NewPlacementSeed)
{
    PlacementSeed = NewPlacementSeed;
    LastObservedPlacementSeed = NewPlacementSeed;
    RespawnMonsters();
}


void AMonsterSpawner::GetCurrentMonsterCounts(
    int32& OutWallCrawlerCount,
    int32& OutFlyingPlatformCount,
    int32& OutFlyingAttackerCount,
    int32& OutTotalCount) const
{
    OutWallCrawlerCount = 0;
    OutFlyingPlatformCount = 0;
    OutFlyingAttackerCount = 0;
    OutTotalCount = 0;

    for (AMonsterBase* Monster : SpawnedMonsters)
    {
        if (!IsValid(Monster))
        {
            continue;
        }

        ++OutTotalCount;

        if (Monster->IsA(AWallCrawler::StaticClass()))
        {
            ++OutWallCrawlerCount;
        }
        else if (Monster->IsA(AFlyingPlatform::StaticClass()))
        {
            ++OutFlyingPlatformCount;
        }
        else if (Monster->IsA(AFlyingAttacker::StaticClass()))
        {
            ++OutFlyingAttackerCount;
        }
    }
}

FString AMonsterSpawner::GetCurrentMonsterCountDebugText() const
{
    int32 WallCount = 0;
    int32 PlatformCount = 0;
    int32 AttackerCount = 0;
    int32 TotalCount = 0;

    GetCurrentMonsterCounts(WallCount, PlatformCount, AttackerCount, TotalCount);

    return FString::Printf(
        TEXT("Monsters: WallCrawler %d/%d | FlyingPlatform %d/%d | FlyingAttacker %d/%d | Total %d/%d"),
        WallCount,
        WallCrawlerCount,
        PlatformCount,
        FlyingPlatformCount,
        AttackerCount,
        FlyingAttackerCount,
        TotalCount,
        WallCrawlerCount + FlyingPlatformCount + FlyingAttackerCount
    );
}


void AMonsterSpawner::ClearSpawnedMonsters()
{
    for (AMonsterBase* Monster : SpawnedMonsters)
    {
        if (IsValid(Monster))
        {
            Monster->Destroy();
        }
    }

    SpawnedMonsters.Empty();
}

void AMonsterSpawner::ClearAllMonsters()
{
    ClearSpawnedMonsters();
    ClearHallucinationGhosts();
}

void AMonsterSpawner::SpawnHallucinationGhosts()
{
    if (!HallucinationGhostClass)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("MonsterSpawner: HallucinationGhostClass 미설정 — BP에서 할당 필요"));
        return;
    }

    // StageHeightMax 결정: 0이면 CliffTotalHeight 자동 사용
    float HeightMax = HallucinationGhostStageHeightMax;
    if (HeightMax <= 0.0f && IsValid(TargetMountain))
    {
        FVector Origin, Extent;
        TargetMountain->GetActorBounds(false, Origin, Extent);
        HeightMax = Origin.Z + Extent.Z;
    }

    TArray<FVector> SpawnedPositions;
    int32 Spawned = 0;

    for (int32 i = 0; i < HallucinationGhostCount; ++i)
    {
        FVector SpawnLoc;
        // SampleFrontBandAirLocation 패턴으로 암벽 앞 공중 위치 샘플
        FRandomStream GhostStream(MakePlacementStreamSeed(EMonsterSpawnKind::FlyingAttacker) + 100000 + i);
        if (!SampleFrontBandAirLocation(GhostStream, SpawnLoc,
            AttackerMinWallDistance, AttackerMinHeightAboveGround,
            AttackerMaxHeightAboveGround, EMonsterSpawnKind::FlyingAttacker))
        {
            continue;
        }

        // 동종 간 최소 거리 체크
        bool bTooClose = false;
        for (const FVector& Pos : SpawnedPositions)
        {
            if (FVector::Dist(SpawnLoc, Pos) < HallucinationGhostMinDistance)
            {
                bTooClose = true;
                break;
            }
        }
        if (bTooClose) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AHallucinationGhost* Ghost = GetWorld()->SpawnActor<AHallucinationGhost>(
            HallucinationGhostClass, SpawnLoc, FRotator::ZeroRotator, Params);

        if (Ghost)
        {
            Ghost->StageHeightMax = HeightMax;
            SpawnedGhosts.Add(Ghost);
            SpawnedPositions.Add(SpawnLoc);
            ++Spawned;
            UE_LOG(LogTemp, Log,
                TEXT("MonsterSpawner: HallucinationGhost 스폰 [%d/%d] @ %s"),
                Spawned, HallucinationGhostCount, *SpawnLoc.ToString());
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("MonsterSpawner: HallucinationGhost 스폰 완료 (%d/%d)"),
        Spawned, HallucinationGhostCount);
}

void AMonsterSpawner::ClearHallucinationGhosts()
{
    for (AActor* Ghost : SpawnedGhosts)
    {
        if (IsValid(Ghost))
        {
            Ghost->Destroy();
        }
    }
    SpawnedGhosts.Empty();
}