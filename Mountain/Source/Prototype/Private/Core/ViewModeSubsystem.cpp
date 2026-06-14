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

    // PIE 직접 시작 시 PostLoadMapWithWorld가 발동하지 않으므로
    // OnPostWorldInitialization으로 추가 커버
    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UViewModeSubsystem::HandleWorldInitialized);
}

void UViewModeSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);

    Super::Deinitialize();
}

void UViewModeSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        return;
    }

    // ��Ű¡�� �ٷ� �Դ� ��찡 ����,
    // PIE �����ʹ� �ʹ� �̸��� �� ������ �� ƽ �̷Ｍ �ٽ� ����
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

    // /Game/Level/FreeRunSetup ���¸� ������ �̸��� ����
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
        (MapName == TEXT("FreeRunSetup") || MapName == TEXT("CliffSelection"))
        ? EViewModeIndex::VMI_Wireframe
        : EViewModeIndex::VMI_Lit;

    ApplyViewMode(TargetMode, false, GEngine->GameViewport->EngineShowFlags);
    GEngine->GameViewport->ViewModeIndex = TargetMode;

    UE_LOG(LogTemp, Warning, TEXT("[ViewModeSubsystem] Map=%s -> Mode=%d"), *MapName, (int32)TargetMode);
}

void UViewModeSubsystem::HandleWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
    if (!World || !World->IsGameWorld())
    {
        return;
    }

    // 다음 틱에 적용 (GameViewport가 아직 준비 안 됐을 수 있으므로)
    ApplyViewModeDeferred(World);
}