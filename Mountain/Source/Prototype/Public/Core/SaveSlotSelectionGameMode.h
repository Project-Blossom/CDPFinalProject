#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SaveSlotSelectionGameMode.generated.h"

/**
 * 세이브 슬롯 선택 화면 전용 GameMode
 */
UCLASS()
class PROTOTYPE_API ASaveSlotSelectionGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASaveSlotSelectionGameMode();

protected:
    virtual void BeginPlay() override;

    // 세이브 슬롯 선택 UI 위젯 클래스
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class USaveSlotSelectionWidget> SaveSlotWidgetClass;

private:
    UPROPERTY()
    TObjectPtr<class USaveSlotSelectionWidget> SaveSlotWidgetInstance;
};
