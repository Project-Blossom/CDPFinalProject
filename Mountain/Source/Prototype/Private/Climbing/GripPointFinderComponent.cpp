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

void UGripPointFinderComponent::CacheClimbableSurfaces()
{
    CachedClimbableSurfaces.Empty();

    if (!IsValid(GetWorld()))
    {
        return;
    }

    // 레벨 내 모든 IClimbableSurface 구현체 찾기 (미래 확장용)
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (IsValid(Actor) && Actor->Implements<UClimbableSurface>())
        {
            TScriptInterface<IClimbableSurface> ClimbableSurface;
            ClimbableSurface.SetObject(Actor);
            ClimbableSurface.SetInterface(Cast<IClimbableSurface>(Actor));
            
            CachedClimbableSurfaces.Add(ClimbableSurface);
            UE_LOG(LogGripFinder, Log, TEXT("Found climbable surface: %s"), *Actor->GetName());
        }
    }
}

// ========================================
// 메인 함수: 그립 포인트 찾기
// ========================================
bool UGripPointFinderComponent::FindGripPoint(const FVector& CameraLocation, const FVector& CameraForward, FGripPointInfo& OutGripInfo)
{
    if (!IsValid(GetWorld()))
    {
        UE_LOG(LogGripFinder, Error, TEXT("World is invalid"));
        return false;
    }

    // ========================================
    // 1단계: 카메라 Ray로 첫 Hit 찾기
    // ========================================
    FHitResult InitialHit;
    if (!FindInitialHitPoint(CameraLocation, CameraForward, InitialHit))
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("No surface found in camera direction"));
        return false;
    }

    // ========================================
    // 2단계: 거리 검증
    // ========================================
    float Distance = FVector::Dist(CameraLocation, InitialHit.Location);
    if (Distance > MaxReachDistance)
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("Surface too far: %.1f cm (max: %.1f cm)"), Distance, MaxReachDistance);
        return false;
    }

    // ========================================
    // 3단계: 인근 평균 경사 계산
    // ========================================
    float AverageSurfaceAngle = 0.0f;
    FVector AverageNormal = FVector::UpVector;
    
    if (!CalculateAverageSurfaceAngle(InitialHit.Location, InitialHit.Normal, AverageSurfaceAngle, AverageNormal))
    {
        UE_LOG(LogGripFinder, Warning, TEXT("Failed to calculate average surface angle"));
        return false;
    }

    // ========================================
    // 4단계: 경사각 검증
    // ========================================
    if (AverageSurfaceAngle < MinSurfaceAngle || AverageSurfaceAngle > MaxSurfaceAngle)
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("Surface angle %.1f° out of range [%.1f - %.1f]"), 
            AverageSurfaceAngle, MinSurfaceAngle, MaxSurfaceAngle);
        return false;
    }

    // ========================================
    // 5단계: 결과 구성
    // ========================================
    OutGripInfo.WorldLocation = InitialHit.Location;
    OutGripInfo.SurfaceNormal = InitialHit.Normal;
    OutGripInfo.AverageNormal = AverageNormal;
    OutGripInfo.SurfaceAngleDegrees = AverageSurfaceAngle;
    OutGripInfo.GripQuality = CalculateGripQuality(AverageSurfaceAngle);
    OutGripInfo.bIsValid = true;

    UE_LOG(LogGripFinder, Log, TEXT("Grip found: Angle=%.1f°, Quality=%.2f, Distance=%.1f cm"), 
        AverageSurfaceAngle, OutGripInfo.GripQuality, Distance);

#if !UE_BUILD_SHIPPING
    // ========================================
    // 디버그 시각화
    // ========================================
    
    // 1. Ray 경로 (노란선)
    DrawDebugLine(
        GetWorld(), 
        CameraLocation, 
        InitialHit.Location, 
        FColor::Yellow, 
        false, 
        2.0f, 
        0, 
        2.0f
    );
    
    // 2. Grip 지점 (녹색 구)
    DrawDebugSphere(
        GetWorld(), 
        OutGripInfo.WorldLocation, 
        15.0f, 
        12, 
        FColor::Green, 
        false, 
        2.0f, 
        0, 
        2.0f
    );
    
    // 3. 경사 샘플링 범위 (노란색 Wire Sphere)
    DrawDebugSphere(
        GetWorld(), 
        OutGripInfo.WorldLocation, 
        SurfaceSampleRadius, 
        16, 
        FColor::Yellow, 
        false, 
        2.0f, 
        0, 
        1.0f
    );
    
    // 4. 평균 Normal 벡터 (파란색 화살표)
    DrawDebugDirectionalArrow(
        GetWorld(), 
        OutGripInfo.WorldLocation, 
        OutGripInfo.WorldLocation + OutGripInfo.AverageNormal * 50.0f, 
        20.0f, 
        FColor::Blue, 
        false, 
        2.0f, 
        0, 
        3.0f
    );
    
    // 5. Hit Normal 벡터 (빨간색 화살표, 비교용)
    DrawDebugDirectionalArrow(
        GetWorld(), 
        OutGripInfo.WorldLocation, 
        OutGripInfo.WorldLocation + OutGripInfo.SurfaceNormal * 40.0f, 
        15.0f, 
        FColor::Red, 
        false, 
        2.0f, 
        0, 
        2.0f
    );
#endif

    return true;
}

// ========================================
// 1단계: 카메라 Ray로 첫 Hit 찾기
// ========================================
bool UGripPointFinderComponent::FindInitialHitPoint(const FVector& Start, const FVector& Direction, FHitResult& OutHit)
{
    if (!IsValid(GetWorld()))
    {
        return false;
    }

    FVector End = Start + Direction * MaxReachDistance;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
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

    // ProceduralMesh만 허용 (MountainGen 지형)
    UProceduralMeshComponent* ProcMesh = Cast<UProceduralMeshComponent>(OutHit.Component.Get());
    if (!IsValid(ProcMesh))
    {
        UE_LOG(LogGripFinder, Verbose, TEXT("Hit non-ProceduralMesh: %s"), *OutHit.Component->GetName());
        return false;
    }

    return true;
}

// ========================================
// 3단계: 인근 평균 경사 계산 (핵심!)
// ========================================
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
    
    for (int32 i = 0; i < NumDirections; i++)
    {
        float Angle = i * AngleStep;
        float Radians = FMath::DegreesToRadians(Angle);
        
        // HitNormal에 수직인 평면에서 방향 계산
        FVector Right = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
        if (Right.IsNearlyZero())
        {
            Right = FVector::CrossProduct(HitNormal, FVector::ForwardVector).GetSafeNormal();
        }
        FVector Up = FVector::CrossProduct(Right, HitNormal).GetSafeNormal();
        
        // 원형 샘플링 방향
        FVector SampleDir = (Right * FMath::Cos(Radians) + Up * FMath::Sin(Radians)).GetSafeNormal();
        FVector SampleOffset = SampleDir * SurfaceSampleRadius;
        
        // HitPoint에서 약간 띄워서 시작 (벽 안쪽에 파묻히지 않도록)
        FVector Start = HitPoint + HitNormal * 5.0f;
        FVector End = HitPoint + SampleOffset;
        
        FHitResult Hit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
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
    for (int32 i = 1; i < SampledNormals.Num(); i++)
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

// ========================================
// 품질 계산: 경사각 기반
// ========================================
float UGripPointFinderComponent::CalculateGripQuality(float SurfaceAngleDegrees) const
{
    // 수직 벽 (90도) = 최고 품질 (1.0)
    // 평평한 바닥/천장 = 낮은 품질

    float DistanceFrom90 = FMath::Abs(SurfaceAngleDegrees - 90.0f);
    float Quality = 1.0f - (DistanceFrom90 / 90.0f);
    
    return FMath::Clamp(Quality, 0.1f, 1.0f); // 최소 0.1
}