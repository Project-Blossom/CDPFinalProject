// File: Source/Prototype/Public/Core/CliffSelectionGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MountainGenSettings.h"
#include "CliffSelectionGameMode.generated.h"

class AMountainGenWorldActor;
class UCliffSelectionLoadingWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllCliffsGenerated);

/**
 * CliffSelection 레벨 GameMode
 * - GameInstance의 CurrentStageIndex 기준으로 난이도 결정 (1 -> Normal, 2 -> Hard)
 * - 3개의 AMountainGenWorldActor를 부채꼴로 스폰
 * - 3개 모두 생성 완료되면 OnAllCliffsGenerated 브로드캐스트 (로딩 UI 제거 트리거)
 */
UCLASS()
class PROTOTYPE_API ACliffSelectionGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACliffSelectionGameMode();

    virtual void BeginPlay() override;

    // 모든 암벽(3개) 생성 완료 시 브로드캐스트
    UPROPERTY(BlueprintAssignable, Category = "CliffSelection")
    FOnAllCliffsGenerated OnAllCliffsGenerated;

    // 스폰된 암벽 액터 3개 (인덱스 0/1/2 = 좌/중/우)
    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    const TArray<AMountainGenWorldActor*>& GetSpawnedCliffs() const { return SpawnedCliffs; }

    // 1회 제한 리롤 — 기존 암벽 Destroy 후 새 Seed로 재생성
    UFUNCTION(BlueprintCallable, Category = "CliffSelection")
    void RerollCliffs();

    // 리롤 사용 여부 (HUD에서 버튼 비활성화 처리에 사용)
    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    bool HasUsedReroll() const { return bRerollUsed; }

    // 3개 암벽 생성이 이미 완료되었는지 여부
    // (GameMode BeginPlay가 Pawn BeginPlay보다 먼저 실행되어 브로드캐스트를 놓치는 경우를 대비)
    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    bool IsAllCliffsGenerated() const { return CompletedCliffCount >= ExpectedCliffCount && ExpectedCliffCount > 0; }

    // [DEAD CODE] 실제 선택 확정 흐름은 ACliffSelectionPawn::OnConfirmSelection에서 처리됨
    // (Seed 저장 -> CurrentStageIndex 증가 -> LoadingUI 플래그 -> OpenLevel 모두 Pawn 쪽에서 수행).
    // 이 GameMode 버전은 어디서도 호출되지 않아 주석 처리함.
    //UFUNCTION(BlueprintCallable, Category = "CliffSelection")
    //void OnConfirmSelection(int32 SelectedCliffIndex);

    // 3개 암벽의 배치 각도 (좌/중/우) - Pawn의 카메라 록온 회전에 사용
    UFUNCTION(BlueprintPure, Category = "CliffSelection")
    const TArray<float>& GetCliffAnglesDeg() const { return CliffAnglesDeg; }

protected:
    // 스폰할 AMountainGenWorldActor 클래스 (BP에서 지정, VoxelMaterial 등 설정된 BP 서브클래스 권장)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Spawn")
    TSubclassOf<AMountainGenWorldActor> CliffActorClass;

    // 카메라(플레이어 시점) 중심으로부터 각 암벽까지의 거리
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Spawn", meta = (ClampMin = "1000.0"))
    float SpawnRadiusCm = 15000.f;

    // 3개 암벽의 배치 각도 (좌/중/우, 도 단위). 인덱스 순서는 SpawnedCliffs와 동일
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|Spawn")
    TArray<float> CliffAnglesDeg = { -45.f, 0.f, 45.f };

    // CliffSelection 로딩 위젯 클래스 (WBP_CliffSelectionLoading)
    // BeginPlay 및 RerollCliffs 시 자동 표시됨
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CliffSelection|UI")
    TSubclassOf<UCliffSelectionLoadingWidget> LoadingWidgetClass;

private:
    // 로딩 위젯 생성 및 뷰포트 표시 (이미 표시 중이면 무시)
    void ShowLoadingWidget();

    UPROPERTY()
    TArray<AMountainGenWorldActor*> SpawnedCliffs;

    UPROPERTY()
    TObjectPtr<UCliffSelectionLoadingWidget> CurrentLoadingWidget;

    int32 CompletedCliffCount = 0;
    int32 ExpectedCliffCount = 0;
    bool bRerollUsed = false;

    // CurrentStageIndex -> EMountainGenDifficulty 매핑
    EMountainGenDifficulty ResolveDifficultyFromStageIndex() const;

    // 3개 암벽 스폰 (Seed는 GameInstance::GeneratedSeeds 사용)
    void SpawnCliffs();

    // 기존 스폰된 암벽 전부 제거
    void DestroySpawnedCliffs();

    // 암벽 1개 생성 완료 콜백
    UFUNCTION()
    void HandleCliffGenerated(AActor* Generator);
};
