#include "Core/StageResultGameMode.h"
#include "UI/StageResultWidget.h"
#include "Core/DownfallGameInstance.h"
#include "Kismet/GameplayStatics.h"

AStageResultGameMode::AStageResultGameMode()
{
    // DefaultPawn을 없애서 캐릭터 스폰 방지
    DefaultPawnClass = nullptr;
}

void AStageResultGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Widget 생성 및 표시
    if (ResultWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            ResultWidgetInstance = CreateWidget<UStageResultWidget>(PC, ResultWidgetClass);
            if (ResultWidgetInstance)
            {
                ResultWidgetInstance->AddToViewport();

                // GameInstance에서 마지막 클리어한 스테이지 정보 가져오기
                UDownfallGameInstance* GI = Cast<UDownfallGameInstance>(GetGameInstance());
                if (GI)
                {
                    // TODO: 어떤 스테이지를 클리어했는지 전달하는 시스템 필요
                    // 지금은 임시로 "Stage1" 하드코딩
                    FName StageId = FName("Stage1");
                    FStageTimeRecord Record = GI->GetStageRecord(StageId);

                    ResultWidgetInstance->SetStageInfo(
                        StageId,
                        Record.LastTime,
                        Record.BestTime,
                        Record.ClearCount
                    );
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ResultWidgetClass not set in StageResultGameMode!"));
    }
}
