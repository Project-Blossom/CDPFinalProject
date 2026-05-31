// File: Source/Prototype/Public/UI/MinimapWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/RetainerBox.h"
#include "MinimapWidget.generated.h"

class UImage;
class UCanvasPanel;
class UCanvasPanelSlot;

/**
 * WBP_Minimap의 C++ 베이스 클래스
 * - MinimapImage: RT_Minimap RenderTarget 표시
 * - PlayerMarker: 플레이어 위치 마커 (기존 AltitudeWidget 마커 재활용)
 * - Color Tint: 날씨 이벤트 시 Image Color Tint로 색조 전환 (방식 2)
 */
UCLASS(Blueprintable, BlueprintType)
class PROTOTYPE_API UMinimapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ── BindWidget — WBP에서 동일 이름으로 위젯 바인딩 필요 ──

    // RetainerBox — MinimapImage를 원형 클립하는 컨테이너
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class URetainerBox> MinimapRetainer;

    // Wireframe 미니맵 이미지 (RT_Minimap RenderTarget 할당)
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> MinimapImage;

    // 플레이어 위치 마커 이미지
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> PlayerMarker;

    // ── C++ 제어 함수 ──────────────────────────────────────────

    /**
     * 미니맵 UV Region 설정 — 스크롤 미니맵 핵심 함수
     * RT_Minimap 전체 중 일부 영역만 표시 (캐릭터 주변 100m 범위)
     * @param UVOffset  표시 시작 UV 좌표 (0~1)
     * @param UVScale   표시 크기 비율 (MinimapViewRange / CliffTotal)
     */
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void SetMinimapUVRegion(FVector2D UVOffset, FVector2D UVScale);

    /**
     * 플레이어 마커 위치 설정
     * 일반 주행: (0.5, 0.5) 중앙 고정
     * 경계 근처: 클램프에 따라 약간 이동
     */
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void SetPlayerMarkerPosition(FVector2D NormalizedPos);

    /**
     * 미니맵 전체 색조 변경 (날씨 이벤트 Lerp에서 호출)
     */
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void SetMinimapTint(const FLinearColor& Tint);

    UFUNCTION(BlueprintPure, Category = "Minimap")
    FVector2D GetMinimapImageSize() const;

protected:
    virtual void NativeConstruct() override;

private:
    // 마커 Canvas Panel Slot 캐시 (SetPosition 최적화)
    UPROPERTY()
    TObjectPtr<UCanvasPanelSlot> PlayerMarkerSlot;
};
