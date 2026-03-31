#include "Monsters/FlyingPlatform.h"
#include "DownfallCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AFlyingPlatform::AFlyingPlatform()
{
}

void AFlyingPlatform::BeginPlay()
{
    Super::BeginPlay();

    // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s (Flying Platform) ready for player"), *GetName());
}

void AFlyingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Behavior Tree가 실행 중이면 기존 로직 스킵
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->BrainComponent && AIController->BrainComponent->IsRunning())
    {
        // Behavior Tree가 제어 중 - 기존 로직 전부 스킵
        return;
    }

// [DISABLED FOR DEMO] Debug visualization: Grab sphere, cross marker, sweep range
#if 0
    FVector GrabLoc = GetGrabLocation();
    DrawDebugSphere(GetWorld(), GrabLoc, 500.0f, 16, bPlayerAttached ? FColor::Yellow : FColor::Green, false, 0.1f, 0, 3.0f);
    DrawDebugLine(GetWorld(), GrabLoc - FVector(50, 0, 0), GrabLoc + FVector(50, 0, 0), FColor::Red, false, 0.1f, 0, 5.0f);
    DrawDebugLine(GetWorld(), GrabLoc - FVector(0, 50, 0), GrabLoc + FVector(0, 50, 0), FColor::Red, false, 0.1f, 0, 5.0f);
    DrawDebugLine(GetWorld(), GrabLoc - FVector(0, 0, 50), GrabLoc + FVector(0, 0, 50), FColor::Red, false, 0.1f, 0, 5.0f);
    DrawDebugSphere(GetWorld(), GrabLoc, 300.0f, 12, FColor::Cyan, false, 0.1f, 0, 2.0f);
#endif

    // 기존 배회 로직 (Behavior Tree 없을 때만)
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
        
        // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s new target: %s"), *GetName(), *CurrentTargetLocation.ToString());
    }

    // 목표로 이동
    FVector CurrentLocation = GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, CurrentTargetLocation);

    if (Distance > ArrivalThreshold)
    {
        // 아직 도착 안 함 - 계속 이동 (암벽 회피 ON)
        FlyToLocation(CurrentTargetLocation, FlightSpeed, true);  // ← 수정: true 추가
    }
    else
    {
        // 도착! - 대기 시작
        if (!bIsIdling)
        {
            bIsIdling = true;
            IdleTimer = 0.0f;
            // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s arrived at target, waiting..."), *GetName());
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
    
    // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s grabbed by player"), *GetName());
}

void AFlyingPlatform::OnPlayerRelease()
{
    bPlayerAttached = false;
    AttachedTime = 0.0f;

    // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s released by player"), *GetName());
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

        // [DISABLED FOR DEMO] UE_LOG(LogMonster, Log, TEXT("%s grip point found at distance: %.1f"), *GetName(), Distance);
        return true;
    }

    return false;
}

FVector AFlyingPlatform::GetGrabLocation() const
{
    // Blueprint에서 추가한 SceneComponent들 중에서 "GrabPoint" 찾기
    TArray<USceneComponent*> SceneComponents;
    GetComponents<USceneComponent>(SceneComponents);
    
    for (USceneComponent* Comp : SceneComponents)
    {
        if (Comp && Comp->GetName().Contains(TEXT("GrabPoint")))
        {
            return Comp->GetComponentLocation();
        }
    }

    // Mesh Socket 체크 (Skeletal Mesh 사용 시)
    if (GetMesh() && GetMesh()->DoesSocketExist(GrabSocketName))
    {
        return GetMesh()->GetSocketLocation(GrabSocketName);
    }

    // 없으면 중심 위치 + 위쪽 오프셋 (50cm)
    return GetActorLocation() + FVector(0, 0, 50.0f);
}

void AFlyingPlatform::OnObstacleDetected(const FVector& ObstacleDirection)
{
    Super::OnObstacleDetected(ObstacleDirection);
    
    // BT 없을 때: 목표 포기
    bHasTarget = false;
    
    // BT 있을 때: Blackboard PatrolLocation 초기화
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
        Blackboard->ClearValue("PatrolLocation");
        
        // [DISABLED FOR DEMO] UE_LOG(LogMonster, Warning, TEXT("%s obstacle detected! Blackboard PatrolLocation cleared"), *GetName());
    }
    else
    {
        // [DISABLED FOR DEMO] UE_LOG(LogMonster, Warning, TEXT("%s obstacle detected! bHasTarget reset"), *GetName());
    }
}