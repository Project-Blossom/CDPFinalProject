#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StageResultGameMode.generated.h"

/**
 * 스테이지 클리어 결과 화면 전용 GameMode
 */
UCLASS()
class PROTOTYPE_API AStageResultGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AStageResultGameMode();

protected:
    virtual void BeginPlay() override;

    // 결과 UI 위젯 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UStageResultWidget> ResultWidgetClass;

private:
    UPROPERTY()
    TObjectPtr<class UStageResultWidget> ResultWidgetInstance;
};
