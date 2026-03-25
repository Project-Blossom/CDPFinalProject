// File: Source/prototype/Private/Climbing/GripPointFinderComponent.cpp
#include "Climbing/GripPointFinderComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY(LogGripFinder);

UGripPointFinderComponent::UGripPointFinderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGripPointFinderComponent::BeginPlay()
{
    Super::BeginPlay();
    CacheClimbableSurfaces();
}

void UGripPointFinderComponent::RefreshClimbableSurfaces()
{
    CacheClimbableSurfaces();
}

void UGripPointFinderComponent::CacheClimbableSurfaces()
{
    CachedClimbableSurfaces.Empty();

    if (!IsValid(GetWorld()))
    {
        return;
    }

    // 레벨 내 모든 IClimbableSurface 구현체 찾기
    // - FlyingPlatform 같은 특수 액터
    // - 설치형 앵커 같은 PlaceActor
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor))
        {
            continue;
        }

        // 자기 자신은 제외
        if (Actor == GetOwner())
        {
            continue;
        }

        if (Actor->Implements<UClimbableSurface>())
        {
            TScriptInterface<IClimbableSurface> ClimbableSurface;
            ClimbableSurface.SetObject(Actor);
            ClimbableSurface.SetInterface(Cast<IClimbableSurface>(Actor));

            CachedClimbableSurfaces.Add(ClimbableSurface);
            UE_LOG(LogGripFinder, Log, TEXT("Found climbable surface: %s"), *Actor->GetName());
        }
    }
}

bool UGripPointFinderComponent::FindGripPoint(const FVector& CameraLocation, const FVector& CameraForward, FGripPointInfo& OutGripInfo)
{
    if (!IsValid(GetWorld()))
    {
        UE_LOG(LogGripFinder, Error, TEXT("World is invalid"));
        return false;
    }

    // 1. 카메라 Ray를 먼저 쏜다
    //    기존 문제:
    //    - IClimbableSurface를 먼저 전부 순회해서
    //      "내가 실제로 바라본 대상"보다 "반경 안의 특수 액터"가 먼저 잡혔음
    FHitResult InitialHit;
    if (!FindInitialHitPoint(CameraLocation, CameraForward, InitialHit))
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("No surface found in camera direction"));

        // 1-1. 레이로 아무것도 못 맞췄을 때만 보조적으로 특수 climbable actor 탐색
        //      (완전 fallback 용도)
        for (const TScriptInterface<IClimbableSurface>& Surface : CachedClimbableSurfaces)
        {
            UObject* SurfaceObject = Surface.GetObject();
            if (!IsValid(SurfaceObject))
            {
                continue;
            }

            FGripPointInfo ActorGripInfo;
            if (IClimbableSurface::Execute_FindNearestGripPoint(
                SurfaceObject,
                CameraLocation,
                SurfaceSampleRadius * 10.0f,
                ActorGripInfo))
            {
                const float Distance = FVector::Dist(CameraLocation, ActorGripInfo.WorldLocation);
                if (Distance <= MaxReachDistance && ActorGripInfo.bIsValid)
                {
                    // BP에서 SourceActor를 안 채웠더라도 여기서 보정
                    if (!IsValid(ActorGripInfo.SourceActor))
                    {
                        ActorGripInfo.SourceActor = Cast<AActor>(SurfaceObject);
                    }

                    OutGripInfo = ActorGripInfo;

#if !UE_BUILD_SHIPPING
                    // 디버그: fallback 특수 그립 감지 시각화
                    DrawDebugLine(
                        GetWorld(),
                        CameraLocation,
                        ActorGripInfo.WorldLocation,
                        FColor::Magenta,
                        false,
                        0.5f,
                        0,
                        3.0f
                    );

                    DrawDebugSphere(
                        GetWorld(),
                        ActorGripInfo.WorldLocation,
                        30.0f,
                        8,
                        FColor::Magenta,
                        false,
                        0.5f,
                        0,
                        2.0f
                    );
#endif

                    UE_LOG(LogGripFinder, Log, TEXT("Fallback climbable grip point: %s at distance %.1f"),
                        *SurfaceObject->GetName(), Distance);
                    return true;
                }
            }
        }

        return false;
    }

    // 2. 레이에 맞은 대상이 IClimbableSurface면 그 액터만 처리
    //    핵심:
    //    - 이제는 "범위 안의 아무 climbable"이 아니라
    //      "내가 실제로 레이로 맞춘 climbable"만 잡는다
    AActor* HitActor = InitialHit.GetActor();
    if (IsValid(HitActor) && HitActor->Implements<UClimbableSurface>())
    {
        FGripPointInfo ActorGripInfo;
        if (IClimbableSurface::Execute_FindNearestGripPoint(
            HitActor,
            InitialHit.Location,         // 카메라 위치가 아니라 실제 맞춘 지점 기준
            SurfaceSampleRadius * 2.0f,  // 앵커/플랫폼 표면 주변 정도만 허용
            ActorGripInfo))
        {
            const float Distance = FVector::Dist(CameraLocation, ActorGripInfo.WorldLocation);
            if (Distance <= MaxReachDistance && ActorGripInfo.bIsValid)
            {
                // BP에서 SourceActor를 안 채웠더라도 여기서 보정
                if (!IsValid(ActorGripInfo.SourceActor))
                {
                    ActorGripInfo.SourceActor = HitActor;
                }

                OutGripInfo = ActorGripInfo;

#if !UE_BUILD_SHIPPING
                // 디버그: 실제 레이로 맞춘 특수 그립 감지 시각화
                DrawDebugLine(
                    GetWorld(),
                    CameraLocation,
                    ActorGripInfo.WorldLocation,
                    FColor::Magenta,
                    false,
                    0.5f,
                    0,
                    3.0f
                );

                DrawDebugSphere(
                    GetWorld(),
                    ActorGripInfo.WorldLocation,
                    30.0f,
                    8,
                    FColor::Magenta,
                    false,
                    0.5f,
                    0,
                    2.0f
                );
#endif

                UE_LOG(LogGripFinder, Log, TEXT("Hit climbable grip point: %s at distance %.1f"),
                    *HitActor->GetName(), Distance);
                return true;
            }
        }

        // 레이로 맞춘 건 climbable actor인데,
        // 그 액터가 유효한 grip point를 못 줬으면 실패 처리
        return false;
    }

    // 3. 일반 ProceduralMesh 지형 처리
    const float Distance = FVector::Dist(CameraLocation, InitialHit.Location);
    if (Distance > MaxReachDistance)
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("Surface too far: %.1f cm (max: %.1f cm)"), Distance, MaxReachDistance);
        return false;
    }

    float AverageSurfaceAngle = 0.0f;
    FVector AverageNormal = FVector::UpVector;

    if (!CalculateAverageSurfaceAngle(InitialHit.Location, InitialHit.Normal, AverageSurfaceAngle, AverageNormal))
    {
        UE_LOG(LogGripFinder, Warning, TEXT("Failed to calculate average surface angle"));
        return false;
    }

    if (AverageSurfaceAngle < MinSurfaceAngle || AverageSurfaceAngle > MaxSurfaceAngle)
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("Surface angle %.1f° out of range [%.1f - %.1f]"),
            AverageSurfaceAngle, MinSurfaceAngle, MaxSurfaceAngle);
        return false;
    }

    OutGripInfo.WorldLocation = InitialHit.Location;
    OutGripInfo.SurfaceNormal = InitialHit.Normal;
    OutGripInfo.AverageNormal = AverageNormal;
    OutGripInfo.SurfaceAngleDegrees = AverageSurfaceAngle;
    OutGripInfo.GripQuality = CalculateGripQuality(AverageSurfaceAngle);
    OutGripInfo.bIsValid = true;
    OutGripInfo.SourceActor = nullptr;     // 일반 지형은 개별 액터를 잡는 게 아님
    OutGripInfo.GripKind = EGripKind::Surface;

    UE_LOG(LogGripFinder, Log, TEXT("Grip found: Angle=%.1f°, Quality=%.2f, Distance=%.1f cm"),
        AverageSurfaceAngle, OutGripInfo.GripQuality, Distance);

    return true;
}

// 카메라 Ray로 hit 탐색
bool UGripPointFinderComponent::FindInitialHitPoint(const FVector& Start, const FVector& Direction, FHitResult& OutHit)
{
    if (!IsValid(GetWorld()))
    {
        return false;
    }

    const FVector End = Start + Direction * MaxReachDistance;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );

    if (!bHit || !OutHit.bBlockingHit)
    {
        return false;
    }

    // IClimbableSurface 구현 액터면 허용
    // - 앵커
    // - FlyingPlatform
    if (AActor* HitActor = OutHit.GetActor())
    {
        if (HitActor->Implements<UClimbableSurface>())
        {
            return true;
        }
    }

    // ProceduralMesh면 일반 지형으로 허용 (MountainGen 지형)
    UProceduralMeshComponent* ProcMesh = Cast<UProceduralMeshComponent>(OutHit.Component.Get());
    if (!IsValid(ProcMesh))
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("Hit unsupported component: %s"), *GetNameSafe(OutHit.Component.Get()));
        return false;
    }

    return true;
}

bool UGripPointFinderComponent::CalculateAverageSurfaceAngle(const FVector& HitPoint, const FVector& HitNormal, float& OutAngleDegrees, FVector& OutAverageNormal)
{
    if (!IsValid(GetWorld()))
    {
        return false;
    }

    // Multi-direction sampling for stable angle calculation
    TArray<FVector> SampledNormals;
    SampledNormals.Add(HitNormal);  // 중심 Normal

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    // 8방향 샘플링 (원형)
    const int32 NumDirections = 8;
    const float AngleStep = 360.0f / NumDirections;

    for (int32 i = 0; i < NumDirections; ++i)
    {
        const float Angle = i * AngleStep;
        const float Radians = FMath::DegreesToRadians(Angle);

        // HitNormal에 수직인 평면에서 방향 계산
        FVector Right = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
        if (Right.IsNearlyZero())
        {
            Right = FVector::CrossProduct(HitNormal, FVector::ForwardVector).GetSafeNormal();
        }

        const FVector Up = FVector::CrossProduct(Right, HitNormal).GetSafeNormal();

        // 원형 샘플링 방향
        const FVector SampleDir = (Right * FMath::Cos(Radians) + Up * FMath::Sin(Radians)).GetSafeNormal();
        const FVector SampleOffset = SampleDir * SurfaceSampleRadius;

        // HitPoint에서 약간 띄워서 시작 (벽 안쪽에 파묻히지 않도록)
        const FVector Start = HitPoint + HitNormal * 5.0f;
        const FVector End = HitPoint + SampleOffset;

        FHitResult Hit;
        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            Start,
            End,
            ECC_Visibility,
            QueryParams
        );

        if (bHit && Hit.bBlockingHit)
        {
            SampledNormals.Add(Hit.Normal);
        }
    }

    // Normal 평균 계산 (중심에 가중치)
    FVector SumNormal = HitNormal * 2.0f;  // 중심 2배 가중치
    for (int32 i = 1; i < SampledNormals.Num(); ++i)
    {
        SumNormal += SampledNormals[i];
    }

    OutAverageNormal = (SumNormal / (SampledNormals.Num() + 1.0f)).GetSafeNormal();

    // 경사각 계산
    float DotProduct = FVector::DotProduct(OutAverageNormal, FVector::UpVector);
    DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
    OutAngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    UE_LOG(LogGripFinder, Verbose, TEXT("Sampled %d normals, Avg Angle: %.1f°"), SampledNormals.Num(), OutAngleDegrees);

    return true;
}

float UGripPointFinderComponent::CalculateGripQuality(float SurfaceAngleDegrees) const
{
    // 수직 벽 (90도) = 최고 품질 (1.0)
    // 평평한 바닥/천장 = 낮은 품질
    const float DistanceFrom90 = FMath::Abs(SurfaceAngleDegrees - 90.0f);
    const float Quality = 1.0f - (DistanceFrom90 / 90.0f);

    return FMath::Clamp(Quality, 0.1f, 1.0f); // 최소 0.1
}