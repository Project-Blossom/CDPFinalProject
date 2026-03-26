#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FreeRunSetupGameMode.generated.h"

/**
 * FreeRun Setup 화면 전용 GameMode
 * - UI 전용 (게임플레이 없음)
 * - FreeRunSetup Widget 자동 생성
 */
UCLASS()
class PROTOTYPE_API AFreeRunSetupGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFreeRunSetupGameMode();

protected:
    virtual void BeginPlay() override;

    // FreeRun Setup Widget 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UFreeRunSetupWidget> FreeRunSetupWidgetClass;

private:
    UPROPERTY()
    TObjectPtr<class UFreeRunSetupWidget> FreeRunSetupWidgetInstance;
};
