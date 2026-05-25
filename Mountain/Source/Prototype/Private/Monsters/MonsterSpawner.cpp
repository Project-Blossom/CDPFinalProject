#include "Monsters/MonsterSpawner.h"

#include "Monsters/MonsterBase.h"
#include "Monsters/WallCrawler.h"
#include "Monsters/FlyingMonster.h"
#include "Monsters/FlyingPlatform.h"
#include "Monsters/FlyingAttacker.h"

#include "MountainGenWorldActor.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"
#include "CollisionShape.h"

AMonsterSpawner::AMonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
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

void AMonsterSpawner::HandleMountainGenerated(AMountainGenWorldActor* Generator)
{
    if (!Generator || Generator != TargetMountain)
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

    const float MinX = FrontX - AllowedDepth;
    const float MaxX = FrontX + 200.0f;

    const FVector Min(
        MinX,
        MountainBounds.Min.Y - 300.0f,
        MountainBounds.Min.Z + 80.0f
    );

    const FVector Max(
        MaxX,
        MountainBounds.Max.Y + 300.0f,
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

    const float SignedFrontDepth = TargetMountain->GetSignedFrontDepthCm(WorldPoint);
    if (SignedFrontDepth < 0.0f)
    {
        return false;
    }

    const float AllowedDepth =
        (FrontSpawnDepthOverrideCm > 0.0f)
        ? FrontSpawnDepthOverrideCm
        : TargetMountain->GetSuggestedFrontSpawnDepthCm();

    return SignedFrontDepth <= AllowedDepth;
}

bool AMonsterSpawner::SampleMountainSurface(FHitResult& OutHit, float MinAbsNormalZ, float MaxAbsNormalZ, ESpawnFailReason* OutFailReason) const
{
    if (OutFailReason)
    {
        *OutFailReason = ESpawnFailReason::NoSurfaceSample;
    }

    FBox Bounds;
    if (!GetMountainBounds(Bounds))
    {
        return false;
    }

    const FVector Center = Bounds.GetCenter();
    const FVector Extent = Bounds.GetExtent() + FVector(BoundsPadding);

    bool bSawFrontBandReject = false;

    for (int32 Attempt = 0; Attempt < 96; ++Attempt)
    {
        FVector Dir = FMath::VRand();
        if (Dir.IsNearlyZero())
        {
            Dir = FVector::ForwardVector;
        }
        Dir.Normalize();

        const FVector Start = Center + FVector(Dir.X * Extent.X, Dir.Y * Extent.Y, Dir.Z * Extent.Z);
        const FVector End = Center - FVector(Dir.X * Extent.X, Dir.Y * Extent.Y, Dir.Z * Extent.Z);

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

bool AMonsterSpawner::SampleFrontBandAirLocation(FVector& OutLocation, float HorizontalRadius, float MinZOffset, float MaxZOffset, EMonsterSpawnKind Kind) const
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
            FMath::FRandRange(FrontBounds.Min.X, FrontBounds.Max.X),
            FMath::FRandRange(FrontBounds.Min.Y, FrontBounds.Max.Y),
            FMath::FRandRange(FrontBounds.Min.Z, FrontBounds.Max.Z)
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

bool AMonsterSpawner::FindWallCrawlerSpawn(FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const
{
    OutResult = FSpawnProbeResult();
    OutFailReason = ESpawnFailReason::None;

    if (!WallCrawlerClass)
    {
        OutFailReason = ESpawnFailReason::NoClass;
        return false;
    }

    const AWallCrawler* WallCDO = WallCrawlerClass->GetDefaultObject<AWallCrawler>();
    const float RequiredTraceDistance = WallCDO ? WallCDO->WallTraceDistance : 100.0f;
    const float RequiredStickDistance = WallCDO ? WallCDO->WallStickDistance : 20.0f;

    FHitResult SurfaceHit;
    if (!SampleMountainSurface(SurfaceHit, WallMinAbsNormalZ, WallMaxAbsNormalZ, &OutFailReason))
    {
        return false;
    }

    const FVector CandidateLocation =
        SurfaceHit.ImpactPoint + SurfaceHit.ImpactNormal * FMath::Max(WallCrawlerSpawnOffset, RequiredStickDistance + 5.0f);

    if (!IsFarEnoughFromSameType(CandidateLocation, EMonsterSpawnKind::WallCrawler))
    {
        OutFailReason = ESpawnFailReason::SameTypeDistance;
        return false;
    }

    if (IsInsideRock(CandidateLocation, 50.0f))
    {
        OutFailReason = ESpawnFailReason::InsideRock;
        return false;
    }

    FHitResult FacingHit;
    const FVector FacingStart = CandidateLocation;
    const FVector FacingEnd = CandidateLocation - SurfaceHit.ImpactNormal * FMath::Max(WallCrawlerFacingProbeDepth, RequiredTraceDistance + 20.0f);

    if (!TraceMountainOnly(FacingStart, FacingEnd, FacingHit))
    {
        OutFailReason = ESpawnFailReason::FacingTraceFailed;
        return false;
    }

    const float FacingAbsNormalZ = FMath::Abs(FacingHit.ImpactNormal.Z);
    if (FacingAbsNormalZ < WallMinAbsNormalZ || FacingAbsNormalZ > WallMaxAbsNormalZ)
    {
        OutFailReason = ESpawnFailReason::FacingNormalInvalid;
        return false;
    }

    OutResult.bSuccess = true;
    OutResult.Location = CandidateLocation;
    OutResult.Rotation = (-FacingHit.ImpactNormal).Rotation() + WallCrawlerVisualRotationOffset;
    OutResult.SurfacePoint = FacingHit.ImpactPoint;
    OutResult.SurfaceNormal = FacingHit.ImpactNormal;
    return true;
}

bool AMonsterSpawner::FindFlyingPlatformSpawn(FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const
{
    OutResult = FSpawnProbeResult();
    OutFailReason = ESpawnFailReason::None;

    if (!FlyingPlatformClass)
    {
        OutFailReason = ESpawnFailReason::NoClass;
        return false;
    }

    const AFlyingPlatform* PlatformCDO = FlyingPlatformClass->GetDefaultObject<AFlyingPlatform>();
    const float PatrolRadius = PlatformCDO ? PlatformCDO->PatrolRadius : 500.0f;
    const float VerticalPatrolRange = PlatformCDO ? PlatformCDO->VerticalPatrolRange : 1000.0f;

    FVector CandidateLocation = FVector::ZeroVector;
    FVector SurfaceNormal = FVector::UpVector;
    FVector SurfacePoint = FVector::ZeroVector;

    if (bRequireCliffProximityForFlying)
    {
        FHitResult SurfaceHit;
        if (!SampleMountainSurface(SurfaceHit, 0.0f, 1.0f, &OutFailReason))
        {
            return false;
        }

        const float WallOffset = FMath::FRandRange(PlatformMinWallDistance, PlatformMaxWallDistance);
        CandidateLocation = SurfaceHit.ImpactPoint + SurfaceHit.ImpactNormal * WallOffset;
        SurfaceNormal = SurfaceHit.ImpactNormal;
        SurfacePoint = SurfaceHit.ImpactPoint;

        if (!PushOutFromWall(CandidateLocation, SurfaceHit.ImpactNormal, PlatformOverlapCheckRadius))
        {
            OutFailReason = ESpawnFailReason::OverlapBlocked;
            return false;
        }
    }
    else
    {
        if (!SampleFrontBandAirLocation(
            CandidateLocation,
            FMath::Max(PlatformClearanceRadius, PatrolRadius * 0.75f),
            PlatformMinHeightAboveGround,
            PlatformMaxHeightAboveGround,
            EMonsterSpawnKind::FlyingPlatform))
        {
            OutFailReason = ESpawnFailReason::NoSurfaceSample;
            return false;
        }
    }

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

    if (IsInsideRock(CandidateLocation, 60.0f))
    {
        OutFailReason = ESpawnFailReason::InsideRock;
        return false;
    }

    if (IsSphereOverlappingWorldStatic(CandidateLocation, PlatformOverlapCheckRadius))
    {
        OutFailReason = ESpawnFailReason::OverlapBlocked;
        return false;
    }

    float HeightAboveGround = 0.0f;
    if (!TraceHeightAboveGround(CandidateLocation, HeightAboveGround))
    {
        OutFailReason = ESpawnFailReason::HeightTraceFailed;
        return false;
    }

    if (HeightAboveGround < PlatformMinHeightAboveGround || HeightAboveGround > PlatformMaxHeightAboveGround)
    {
        OutFailReason = ESpawnFailReason::HeightOutOfRange;
        return false;
    }

    const float NeededRadius = FMath::Max(PlatformClearanceRadius, PatrolRadius * 0.75f);
    const float NeededHalfHeight = FMath::Max(PlatformClearanceHalfHeight, VerticalPatrolRange * 0.25f);

    if (!HasLocalClearance(CandidateLocation, NeededRadius, NeededHalfHeight))
    {
        OutFailReason = ESpawnFailReason::ClearanceFailed;
        return false;
    }

    OutResult.bSuccess = true;
    OutResult.Location = CandidateLocation;
    OutResult.Rotation = FRotator::ZeroRotator;
    OutResult.SurfacePoint = SurfacePoint.IsNearlyZero() ? CandidateLocation : SurfacePoint;
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

bool AMonsterSpawner::FindFlyingAttackerSpawn(FSpawnProbeResult& OutResult, ESpawnFailReason& OutFailReason) const
{
    OutResult = FSpawnProbeResult();
    OutFailReason = ESpawnFailReason::None;

    if (!FlyingAttackerClass)
    {
        OutFailReason = ESpawnFailReason::NoClass;
        return false;
    }

    const AFlyingAttacker* AttackerCDO = FlyingAttackerClass->GetDefaultObject<AFlyingAttacker>();
    const float TerritoryRadius = AttackerCDO ? AttackerCDO->TerritoryRadius : 1000.0f;
    const float SafeDistance = AttackerCDO ? AttackerCDO->SafeDistance : 500.0f;

    FVector CandidateLocation = FVector::ZeroVector;

    if (bRequireCliffProximityForFlying)
    {
        FHitResult SurfaceHit;
        if (!SampleMountainSurface(SurfaceHit, 0.0f, 1.0f, &OutFailReason))
        {
            return false;
        }

        const float WallOffset = FMath::FRandRange(AttackerMinWallDistance, AttackerMaxWallDistance);
        CandidateLocation = SurfaceHit.ImpactPoint + SurfaceHit.ImpactNormal * WallOffset;
    }
    else
    {
        if (!SampleFrontBandAirLocation(
            CandidateLocation,
            FMath::Max(AttackerLocalClearanceRadius, TerritoryRadius * 0.35f),
            AttackerMinHeightAboveGround,
            AttackerMaxHeightAboveGround,
            EMonsterSpawnKind::FlyingAttacker))
        {
            OutFailReason = ESpawnFailReason::NoSurfaceSample;
            return false;
        }
    }

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

    if (IsInsideRock(CandidateLocation, 60.0f))
    {
        OutFailReason = ESpawnFailReason::InsideRock;
        return false;
    }

    float HeightAboveGround = 0.0f;
    if (!TraceHeightAboveGround(CandidateLocation, HeightAboveGround))
    {
        OutFailReason = ESpawnFailReason::HeightTraceFailed;
        return false;
    }

    if (HeightAboveGround < AttackerMinHeightAboveGround || HeightAboveGround > AttackerMaxHeightAboveGround)
    {
        OutFailReason = ESpawnFailReason::HeightOutOfRange;
        return false;
    }

    if (!HasLocalClearance(CandidateLocation, AttackerLocalClearanceRadius, AttackerLocalClearanceHalfHeight))
    {
        OutFailReason = ESpawnFailReason::ClearanceFailed;
        return false;
    }

    if (!HasLocalClearance(
        CandidateLocation,
        FMath::Max(AttackerLocalClearanceRadius, SafeDistance * 0.6f),
        FMath::Max(AttackerLocalClearanceHalfHeight, 180.0f)))
    {
        OutFailReason = ESpawnFailReason::ClearanceFailed;
        return false;
    }

    if (!EvaluateFlyingAttackerTerritory(CandidateLocation, TerritoryRadius))
    {
        OutFailReason = ESpawnFailReason::TerritoryFailed;
        return false;
    }

    OutResult.bSuccess = true;
    OutResult.Location = CandidateLocation;
    OutResult.Rotation = FRotator::ZeroRotator;
    OutResult.SurfacePoint = CandidateLocation;
    OutResult.SurfaceNormal = FVector::UpVector;
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

    ClearAllMonsters();

    int32 SpawnedWall = 0;
    int32 SpawnedPlatform = 0;
    int32 SpawnedAttacker = 0;

    FSpawnFailStats WallStats;
    FSpawnFailStats PlatformStats;
    FSpawnFailStats AttackerStats;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (WallCrawlerClass)
    {
        for (int32 Attempt = 0; Attempt < MaxAttemptsPerWallCrawler && SpawnedWall < WallCrawlerCount; ++Attempt)
        {
            FSpawnProbeResult Result;
            ESpawnFailReason FailReason = ESpawnFailReason::None;

            if (!FindWallCrawlerSpawn(Result, FailReason))
            {
                WallStats.Add(FailReason);
                continue;
            }

            AWallCrawler* Spawned = GetWorld()->SpawnActor<AWallCrawler>(
                WallCrawlerClass,
                Result.Location,
                Result.Rotation,
                SpawnParams);

            if (!Spawned)
            {
                WallStats.Add(ESpawnFailReason::SpawnActorFailed);
                continue;
            }

            SpawnedMonsters.Add(Spawned);
            SpawnedWall++;

            if (bShowDebugPoints)
            {
                DrawDebugSphere(GetWorld(), Result.Location, 35.0f, 12, FColor::Green, false, 12.0f, 0, 2.0f);
                DrawDebugLine(GetWorld(), Result.Location, Result.SurfacePoint, FColor::Green, false, 12.0f, 0, 2.0f);
            }
        }
    }

    if (FlyingPlatformClass)
    {
        for (int32 Attempt = 0; Attempt < MaxAttemptsPerFlyingPlatform && SpawnedPlatform < FlyingPlatformCount; ++Attempt)
        {
            FSpawnProbeResult Result;
            ESpawnFailReason FailReason = ESpawnFailReason::None;

            if (!FindFlyingPlatformSpawn(Result, FailReason))
            {
                PlatformStats.Add(FailReason);
                continue;
            }

            AFlyingPlatform* Spawned = GetWorld()->SpawnActor<AFlyingPlatform>(
                FlyingPlatformClass,
                Result.Location,
                Result.Rotation,
                SpawnParams);

            if (!Spawned)
            {
                PlatformStats.Add(ESpawnFailReason::SpawnActorFailed);
                continue;
            }

            SpawnedMonsters.Add(Spawned);
            SpawnedPlatform++;

            if (bShowDebugPoints)
            {
                DrawDebugSphere(GetWorld(), Result.Location, 35.0f, 12, FColor::Blue, false, 12.0f, 0, 2.0f);
                DrawDebugLine(GetWorld(), Result.SurfacePoint, Result.Location, FColor::Blue, false, 12.0f, 0, 2.0f);
            }
        }
    }

    if (FlyingAttackerClass)
    {
        for (int32 Attempt = 0; Attempt < MaxAttemptsPerFlyingAttacker && SpawnedAttacker < FlyingAttackerCount; ++Attempt)
        {
            FSpawnProbeResult Result;
            ESpawnFailReason FailReason = ESpawnFailReason::None;

            if (!FindFlyingAttackerSpawn(Result, FailReason))
            {
                AttackerStats.Add(FailReason);
                continue;
            }

            AFlyingAttacker* Spawned = GetWorld()->SpawnActor<AFlyingAttacker>(
                FlyingAttackerClass,
                Result.Location,
                Result.Rotation,
                SpawnParams);

            if (!Spawned)
            {
                AttackerStats.Add(ESpawnFailReason::SpawnActorFailed);
                continue;
            }

            SpawnedMonsters.Add(Spawned);
            SpawnedAttacker++;

            if (bShowDebugPoints)
            {
                DrawDebugSphere(GetWorld(), Result.Location, 40.0f, 12, FColor::Red, false, 12.0f, 0, 2.0f);
                DrawDebugLine(GetWorld(), Result.SurfacePoint, Result.Location, FColor::Red, false, 12.0f, 0, 2.0f);
            }
        }
    }

    LogFailStats(TEXT("[MonsterSpawner][WallCrawler]"), WallStats, SpawnedWall, WallCrawlerCount);
    LogFailStats(TEXT("[MonsterSpawner][FlyingPlatform]"), PlatformStats, SpawnedPlatform, FlyingPlatformCount);
    LogFailStats(TEXT("[MonsterSpawner][FlyingAttacker]"), AttackerStats, SpawnedAttacker, FlyingAttackerCount);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[MonsterSpawner] Result | Wall %d/%d | Platform %d/%d | Attacker %d/%d | Total %d"),
        SpawnedWall, WallCrawlerCount,
        SpawnedPlatform, FlyingPlatformCount,
        SpawnedAttacker, FlyingAttackerCount,
        SpawnedMonsters.Num()
    );
}

void AMonsterSpawner::ClearAllMonsters()
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