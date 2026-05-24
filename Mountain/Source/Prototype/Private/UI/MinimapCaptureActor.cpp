// MinimapCaptureActor.cpp
#include "UI/MinimapCaptureActor.h"

#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogMinimapCapture, Log, All);

AMinimapCaptureActor::AMinimapCaptureActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMinimapCaptureActor::BeginPlay()
{
    Super::BeginPlay();

    // 암벽이 BeginPlay 시점에 이미 존재하므로 즉시 캡처
    TriggerCapture();
}

void AMinimapCaptureActor::TriggerCapture()
{
    TArray<AActor*> FoundCaptures;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(), ASceneCapture2D::StaticClass(), FoundCaptures);

    if (FoundCaptures.Num() == 0)
    {
        UE_LOG(LogMinimapCapture, Warning,
            TEXT("MinimapCaptureActor: SceneCapture2D를 찾을 수 없습니다."));
        return;
    }

    ASceneCapture2D* CaptureActor = Cast<ASceneCapture2D>(FoundCaptures[0]);
    if (!CaptureActor) return;

    USceneCaptureComponent2D* CaptureComp = CaptureActor->GetCaptureComponent2D();
    if (!CaptureComp) return;

    if (bForceWireframeShowFlag)
    {
        CaptureComp->ShowFlags.SetWireframe(true);
        UE_LOG(LogMinimapCapture, Log,
            TEXT("MinimapCaptureActor: ShowFlags.SetWireframe(true) 적용"));
    }

    CaptureComp->CaptureScene();

    UE_LOG(LogMinimapCapture, Log,
        TEXT("MinimapCaptureActor: 캡처 완료 (%s) [WireframeShowFlag=%s]"),
        *CaptureActor->GetName(),
        bForceWireframeShowFlag ? TEXT("true") : TEXT("false(레벨ViewMode 의존)"));
}
