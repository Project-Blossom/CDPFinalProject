// File: Source/Prototype/Public/Core/EndingGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EndingGameMode.generated.h"

class UEndingSequenceWidget;

/**
 * Ending 레벨 전용 GameMode (기획서 v2).
 * BeginPlay에서 EndingSequenceWidgetClass(WBP_Ending)를 생성해 뷰포트에 추가한다.
 * 그 이후 흐름(이미지 1~4 클릭 전환, 결과 화면 전환, 메인메뉴 복귀)은
 * UEndingSequenceWidget / UEndingResultWidget이 자체적으로 처리한다.
 */
UCLASS()
class PROTOTYPE_API AEndingGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEndingGameMode();

    virtual void BeginPlay() override;

protected:
    // Ending 컷씬 시퀀스 위젯 클래스 (WBP_Ending). Class Defaults에서 지정.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ending|UI")
    TSubclassOf<UEndingSequenceWidget> EndingSequenceWidgetClass;

private:
    UPROPERTY()
    TObjectPtr<UEndingSequenceWidget> CurrentSequenceWidget;

    void ShowEndingSequenceWidget();
};
