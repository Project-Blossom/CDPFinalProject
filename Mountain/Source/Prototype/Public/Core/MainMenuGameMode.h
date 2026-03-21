#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 * 메인 메뉴 전용 GameMode
 */
UCLASS()
class PROTOTYPE_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMainMenuGameMode();

protected:
    virtual void BeginPlay() override;

    // 메인 메뉴 UI 위젯 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass;

private:
    UPROPERTY()
    TObjectPtr<class UMainMenuWidget> MainMenuWidgetInstance;
};
