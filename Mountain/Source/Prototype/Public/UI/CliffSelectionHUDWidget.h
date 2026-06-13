// File: Source/Prototype/Public/UI/CliffSelectionHUDWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CliffSelectionHUDWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * CliffSelection 정보 / 리롤 UI (WBP_CliffSelectionHUD)
 * - 현재 록온된 암벽의 Seed / 난이도 표시
 * - 리롤 버튼 (1회 제한)
 * - 표시 갱신은 카메라/Pawn 쪽에서 UpdateSelectionInfo() 호출로 수행
 */
UCLASS()
class PROTOTYPE_API UCliffSelectionHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 카메라가 암벽을 전환하거나 처음 록온할 때 호출
    // CliffIndex: 0/1/2 (좌/중/우), Seed: 해당 암벽의 Seed, DifficultyText: 표시용 난이도 문자열
    UFUNCTION(BlueprintCallable, Category = "CliffSelection|HUD")
    void UpdateSelectionInfo(int32 CliffIndex, int32 Seed, const FString& DifficultyText);

    // 좌/우 방향키 입력 시 안내 텍스트 표시/숨김 등에 사용 (필요 시)
    UFUNCTION(BlueprintCallable, Category = "CliffSelection|HUD")
    void SetInputHintVisible(bool bVisible);

protected:
    virtual void NativeConstruct() override;

    // 리롤 버튼
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> RerollButton;

    // 현재 암벽 Seed 표시
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> SeedText;

    // 현재 암벽 난이도 표시
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> DifficultyText;

    // 입력 안내 텍스트 (← → 이동 / Enter 선택 / R 리롤)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> InputHintText;

private:
    UFUNCTION()
    void HandleRerollClicked();

    // 리롤 1회 사용 후 버튼 비활성화 + 색상 비활성화 처리
    void SetRerollButtonUsed();
};
