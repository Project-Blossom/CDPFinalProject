#pragma once

#include "Modules/ModuleManager.h"

class FSimpleMeshPlacerModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};