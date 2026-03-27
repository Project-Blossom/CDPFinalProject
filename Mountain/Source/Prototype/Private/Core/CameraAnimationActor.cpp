#include "Core/CameraAnimationActor.h"
#include "Camera/CameraActor.h"
#include "MountainGenWorldActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ACameraAnimationActor::ACameraAnimationActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentDistance = 0.0f;
    bInitialized = false;
}

void ACameraAnimationActor::BeginPlay()
{
    Super::BeginPlay();
    
    Initialize();
}

void ACameraAnimationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bInitialized || !CameraActor)
        return;

    // 위로 이동
    float MoveDistance = CameraSpeed * DeltaTime;
    CurrentDistance += MoveDistance;

    FVector NewLocation = CameraActor->GetActorLocation();
    NewLocation.Z += MoveDistance;
    CameraActor->SetActorLocation(NewLocation);

    // 목표 높이에 도달하면 재시작
    if (CurrentDistance >= TargetMoveDistance)
    {
        Restart();
    }
}

void ACameraAnimationActor::Initialize()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraAnimationActor - World is null"));
        return;
    }

    // 카메라 Actor 찾기
    for (TActorIterator<ACameraActor> It(World); It; ++It)
    {
        CameraActor = *It;
        UE_LOG(LogTemp, Warning, TEXT("CameraAnimationActor - Camera found: %s"), *CameraActor->GetName());
        break;
    }

    if (!CameraActor)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraAnimationActor - Camera Actor not found!"));
        return;
    }

    // MountainGen Actor 찾기
    for (TActorIterator<AMountainGenWorldActor> It(World); It; ++It)
    {
        MountainActor = *It;
        UE_LOG(LogTemp, Warning, TEXT("CameraAnimationActor - MountainGen found: %s"), *MountainActor->GetName());
        break;
    }

    if (!MountainActor)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraAnimationActor - MountainGen Actor not found!"));
        return;
    }

    // 시작 위치 저장
    StartLocation = CameraActor->GetActorLocation();
    
    // 기본 이동 거리를 CliffHeightCm으로 설정 (에디터에서 수정 가능)
    if (TargetMoveDistance == 80000.0f) // 기본값이면
    {
        TargetMoveDistance = MountainActor->Settings.CliffHeightCm;
        UE_LOG(LogTemp, Warning, TEXT("  TargetMoveDistance set from CliffHeightCm: %.0f cm"), TargetMoveDistance);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CameraAnimationActor - Initialized!"));
    UE_LOG(LogTemp, Warning, TEXT("  Start Location: %s"), *StartLocation.ToString());
    UE_LOG(LogTemp, Warning, TEXT("  Target Move Distance: %.0f cm (%.0f m)"), TargetMoveDistance, TargetMoveDistance / 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("  Camera Speed: %.0f cm/s (%.0f m/s)"), CameraSpeed, CameraSpeed / 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("  Estimated Loop Time: %.1f seconds"), TargetMoveDistance / CameraSpeed);

    bInitialized = true;
}

void ACameraAnimationActor::Restart()
{
    if (!CameraActor)
        return;

    UE_LOG(LogTemp, Log, TEXT("CameraAnimationActor - Restarting loop!"));

    // 시작 위치로 리셋
    CameraActor->SetActorLocation(StartLocation);
    
    // 거리 초기화
    CurrentDistance = 0.0f;
}
