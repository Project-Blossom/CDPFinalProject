#include "Core/ViewModeSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/EngineBaseTypes.h"
#include "ShowFlags.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UViewModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UViewModeSubsystem::HandlePostLoadMap);
}

void UViewModeSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    Super::Deinitialize();
}

void UViewModeSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        return;
    }

    // 패키징은 바로 먹는 경우가 많고,
    // PIE 에디터는 너무 이르면 안 먹으니 한 틱 미뤄서 다시 적용
    ApplyViewModeForWorld(LoadedWorld);
    ApplyViewModeDeferred(LoadedWorld);
}

void UViewModeSubsystem::ApplyViewModeDeferred(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        return;
    }

    FTimerDelegate RetryDelegate;
    RetryDelegate.BindUObject(this, &UViewModeSubsystem::ApplyViewModeForWorld, LoadedWorld);

    LoadedWorld->GetTimerManager().SetTimerForNextTick(RetryDelegate);
}

void UViewModeSubsystem::ApplyViewModeForWorld(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        return;
    }

    if (!GEngine || !GEngine->GameViewport)
    {
        return;
    }

    FString MapName = LoadedWorld->GetMapName();

    // /Game/Level/FreeRunSetup 형태면 마지막 이름만 추출
    int32 SlashIndex = INDEX_NONE;
    if (MapName.FindLastChar(TEXT('/'), SlashIndex))
    {
        MapName = MapName.Mid(SlashIndex + 1);
    }

    // PIE: UEDPIE_0_FreeRunSetup -> FreeRunSetup
    if (MapName.StartsWith(TEXT("UEDPIE_")))
    {
        int32 FirstUnderscore = INDEX_NONE;
        int32 SecondUnderscore = INDEX_NONE;

        if (MapName.FindChar(TEXT('_'), FirstUnderscore))
        {
            SecondUnderscore = MapName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromStart, FirstUnderscore + 1);
            if (SecondUnderscore != INDEX_NONE)
            {
                MapName = MapName.Mid(SecondUnderscore + 1);
            }
        }
    }

    const EViewModeIndex TargetMode =
        (MapName == TEXT("FreeRunSetup"))
        ? EViewModeIndex::VMI_Wireframe
        : EViewModeIndex::VMI_Lit;

    ApplyViewMode(TargetMode, false, GEngine->GameViewport->EngineShowFlags);
    GEngine->GameViewport->ViewModeIndex = TargetMode;

    UE_LOG(LogTemp, Warning, TEXT("[ViewModeSubsystem] Map=%s -> Mode=%d"), *MapName, (int32)TargetMode);
}