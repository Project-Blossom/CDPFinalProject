#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UIButtonClickSoundRouter.generated.h"

class UDownfallGameInstance;

UCLASS()
class PROTOTYPE_API UUIButtonClickSoundRouter : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UDownfallGameInstance* InGameInstance);

    UFUNCTION()
    void HandleButtonClicked();

private:
    UPROPERTY(Transient)
    TObjectPtr<UDownfallGameInstance> GameInstance = nullptr;
};
