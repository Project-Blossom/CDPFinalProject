#include "Monsters/FlyingPlatform.h"
#include "DownfallCharacter.h"
#include "Components/SkeletalMeshComponent.h"

AFlyingPlatform::AFlyingPlatform()
{
}

void AFlyingPlatform::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMonster, Log, TEXT("%s (Flying Platform) ready for player"), *GetName());
}

void AFlyingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPlayerAttached)
    {
        AttachedTime += DeltaTime;

        if (AttachedTime >= DescendStartTime)
        {
            // 하강
            FVector CurrentLocation = GetActorLocation();
            FVector NewLocation = CurrentLocation + FVector(0, 0, -DescendSpeed * DeltaTime);
            SetActorLocation(NewLocation);
        }
        else
        {
            // 탑승 중에도 배회
            UpdatePatrol(DeltaTime);
        }
    }
    else
    {
        // 일반 배회
        UpdatePatrol(DeltaTime);
    }
}

void AFlyingPlatform::UpdatePatrol(float DeltaTime)
{
    // 목표가 없으면 새로 설정
    if (!bHasTarget)
    {
        CurrentTargetLocation = GetRandomPatrolLocation();
        bHasTarget = true;
        bIsIdling = false;
        IdleTimer = 0.0f;
        
        UE_LOG(LogMonster, Log, TEXT("%s new target: %s"), *GetName(), *CurrentTargetLocation.ToString());
    }

    // 목표로 이동
    FVector CurrentLocation = GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, CurrentTargetLocation);

    if (Distance > ArrivalThreshold)
    {
        // 아직 도착 안 함 - 계속 이동
        FlyToLocation(CurrentTargetLocation, FlightSpeed);
    }
    else
    {
        // 도착! - 대기 시작
        if (!bIsIdling)
        {
            bIsIdling = true;
            IdleTimer = 0.0f;
            UE_LOG(LogMonster, Log, TEXT("%s arrived at target, waiting..."), *GetName());
        }

        // 대기 중
        IdleTimer += DeltaTime;
        
        if (IdleTimer >= IdleWaitTime)
        {
            // 대기 끝 - 다음 목표 설정
            bHasTarget = false;
        }
    }
}

void AFlyingPlatform::OnPlayerGrab(ADownfallCharacter* Player)
{
    if (!Player) return;

    bPlayerAttached = true;
    AttachedTime = 0.0f;
    
    UE_LOG(LogMonster, Log, TEXT("%s grabbed by player"), *GetName());
}

void AFlyingPlatform::OnPlayerRelease()
{
    bPlayerAttached = false;
    AttachedTime = 0.0f;

    UE_LOG(LogMonster, Log, TEXT("%s released by player"), *GetName());
}

bool AFlyingPlatform::FindNearestGripPoint_Implementation(const FVector& SearchOrigin, float SearchRadius, FGripPointInfo& OutGripInfo)
{
    FVector GripLocation = GetGrabLocation();
    float Distance = FVector::Dist(SearchOrigin, GripLocation);

    if (Distance <= SearchRadius)
    {
        OutGripInfo.WorldLocation = GripLocation;
        OutGripInfo.SurfaceNormal = FVector::UpVector;
        OutGripInfo.GripQuality = 1.0f;
        OutGripInfo.SurfaceAngleDegrees = 0.0f;
        OutGripInfo.AverageNormal = FVector::UpVector;
        OutGripInfo.bIsValid = true;

        UE_LOG(LogMonster, Log, TEXT("%s grip point found at distance: %.1f"), *GetName(), Distance);
        return true;
    }

    return false;
}

FVector AFlyingPlatform::GetGrabLocation() const
{
    if (GetMesh() && GetMesh()->DoesSocketExist(GrabSocketName))
    {
        return GetMesh()->GetSocketLocation(GrabSocketName);
    }

    return GetActorLocation();
}