#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FreeRunGameMode.generated.h"

/**
 * FreeRun 레벨 전용 GameMode
 * - URL 파라미터에서 Seed, Difficulty 파싱
 * - MountainGenWorldActor에 설정 적용
 */
UCLASS()
class PROTOTYPE_API AFreeRunGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFreeRunGameMode();

protected:
    virtual void BeginPlay() override;

private:
    // URL에서 Seed와 Difficulty 파싱
    void ParseURLParameters();

    // MountainGenWorldActor를 찾아서 설정 적용
    void ApplyMountainSettings(int32 Seed, int32 Difficulty);

    int32 ParsedSeed = 0;
    int32 ParsedDifficulty = 1; // 기본값 Normal
};
