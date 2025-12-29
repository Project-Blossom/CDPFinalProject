#pragma once

#include "Modules/ModuleManager.h"

class FMountainGenModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};