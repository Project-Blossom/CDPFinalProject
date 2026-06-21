#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndingResultWidget.generated.h"

/**
 * Ending 레벨 결과 화면 (기획서 v2 — WBP_EndingResult 대응 C++ 베이스 클래스)
 *
 * 표시 내용:
 *  - 이번 회차 Stage1/2/3 클리어 타임
 *  - 클리어 타임이 3개 SaveSlot 통틀어 그 스테이지의 최고 기록과 같으면 NEW 배지 표시
 *    (기획서 "신기록 판정 기준" 옵션 A — 스테이지별 개별 비교. 기존 SaveGame 데이터
 *    구조(FStageTimeRecord.BestTime이 스테이지별로 따로 관리됨)와 자연스럽게 맞아서
 *    옵션 A로 확정)
 *  - 3개 SaveSlot 통틀어 가장 짧은 합산 클리어 타임("전체 베스트 기록") 한 줄 표시
 *  - 메인메뉴 버튼 클릭 시 MainMenu 레벨로 전환
 */
UCLASS()
class PROTOTYPE_API UEndingResultWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // GameInstance 데이터를 읽어 화면을 채운다. NativeConstruct에서 1회 자동 호출되지만,
    // 필요하면 Blueprint에서 다시 호출해 갱신할 수 있다(예: 슬롯 전환 직후 등).
    UFUNCTION(BlueprintCallable, Category = "Ending Result")
    void PopulateResults();

    UFUNCTION(BlueprintCallable, Category = "Ending Result")
    void OnMainMenuClicked();

protected:
    virtual void NativeConstruct() override;

    // ── Stage별 클리어 타임 텍스트 (필수) ──
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* Stage1TimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* Stage2TimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* Stage3TimeText;

    // ── Stage별 NEW 배지 (선택 — 디자인에 없으면 비워둬도 됨) ──
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Stage1NewText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Stage2NewText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Stage3NewText;

    // ── Stage별 최고 기록(3개 SaveSlot 통틀어) 텍스트 (선택) ──
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Stage1BestTimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Stage2BestTimeText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* Stage3BestTimeText;

    // ── 전체 베스트 기록 (SaveSlot 3개 통틀어 가장 짧은 합산 클리어 타임 + 슬롯 번호) ──
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UTextBlock* OverallBestText;

    // ── 메인메뉴 버튼 (필수) ──
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* MainMenuButton;

    // 이 화면이 뜰 때 재생할 FadeIn 애니메이션(선택). 기획서 "WBP_EndingResult FadeIn
    // (1초)" 요구사항용 — 디자이너가 위젯 애니메이션 이름을 FadeInAnim으로 만들면
    // NativeConstruct에서 자동 재생된다. 없어도 동작에는 지장 없다.
    UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnimOptional))
    class UWidgetAnimation* FadeInAnim;

    // 이 위젯이 표시할 3개 스테이지의 StageId.
    // DownfallGameMode::CurrentStageId에 실제로 쓰이는 값(레벨 Blueprint 기본값)과
    // 정확히 일치해야 한다 — 다르면 GetStageRecord/GetBestStageTimeAcrossAllSlots가
    // 빈 기록을 반환한다.
    // [DEBUG-FIX] 기본값을 레벨 파일명 규칙(Stage_1/Stage_2/Stage_3, 언더스코어 포함)에
    // 맞춰 추측해 둔 값이다. 실제 값은 각 Stage 레벨의 GameMode Blueprint Class Defaults
    // → Current Stage Id 에서 직접 확인 후, 다르면 WBP_EndingResult Class Defaults에서
    // 이 값을 덮어써야 한다(C++ 기본값만으로는 보장 못 함).
    UPROPERTY(EditDefaultsOnly, Category = "Ending Result")
    FName Stage1Id = FName("Stage_1");

    UPROPERTY(EditDefaultsOnly, Category = "Ending Result")
    FName Stage2Id = FName("Stage_2");

    UPROPERTY(EditDefaultsOnly, Category = "Ending Result")
    FName Stage3Id = FName("Stage_3");

    // 메인메뉴 레벨 이름
    UPROPERTY(EditDefaultsOnly, Category = "Ending Result")
    FName MainMenuLevel = FName("MainMenu");

private:
    void SetStageRow(class UTextBlock* TimeText, class UTextBlock* NewText, class UTextBlock* BestTimeText, FName StageId);

    UFUNCTION()
    void HandleMainMenuClicked();

    static FText FormatClearTime(float Seconds);
};
