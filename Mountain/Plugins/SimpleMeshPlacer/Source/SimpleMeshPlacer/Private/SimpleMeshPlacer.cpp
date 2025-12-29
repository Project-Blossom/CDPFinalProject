#include "SimpleMeshPlacer.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FSimpleMeshPlacerModule, SimpleMeshPlacer)

void FSimpleMeshPlacerModule::StartupModule()
{
    // 플러그인 켜질 때 호출 ? 지금은 할 일 없음
}

void FSimpleMeshPlacerModule::ShutdownModule()
{
    // 플러그인 꺼질 때 호출 ? 지금은 할 일 없음
}