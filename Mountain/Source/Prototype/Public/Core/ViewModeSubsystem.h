#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ViewModeSubsystem.generated.h"

UCLASS()
class PROTOTYPE_API UViewModeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    void HandlePostLoadMap(UWorld* LoadedWorld);
    void ApplyViewModeForWorld(UWorld* LoadedWorld);
    void ApplyViewModeDeferred(UWorld* LoadedWorld);
};