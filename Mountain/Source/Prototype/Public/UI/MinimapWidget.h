// File: Source/Prototype/Public/UI/MinimapWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

    // Wireframe 미니맵 이미지 (RT_Minimap RenderTarget 할당)
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> MinimapImage;

    // 플레이어 위치 마커 이미지
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> PlayerMarker;

    // ── C++ 제어 함수 ──────────────────────────────────────────

    /**
     * 미니맵 전체 색조 변경 (날씨 이벤트 Lerp에서 호출)
     * 방식 2: UMG Image Color Tint 곱셈 적용
     * - 정상: (1,1,1,1) 원본 색상 유지
     * - Blizzard: (0.6, 0.8, 1.0) 차가운 파란색
     * - BloodMoon: (1.0, 0.2, 0.1) 붉은색
     */
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void SetMinimapTint(const FLinearColor& Tint);

    /**
     * 플레이어 마커 위치 설정
     * @param UV (0~1) 범위의 미니맵 내 UV 좌표
     *           U: 수평 위치, V: 수직 위치 (0=상단, 1=하단)
     */
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void SetPlayerMarkerUV(FVector2D UV);

    /**
     * 미니맵 이미지 크기 반환 (UV → 픽셀 좌표 계산용)
     */
    UFUNCTION(BlueprintPure, Category = "Minimap")
    FVector2D GetMinimapImageSize() const;

protected:
    virtual void NativeConstruct() override;

private:
    // 마커 Canvas Panel Slot 캐시 (SetPosition 최적화)
    UPROPERTY()
    TObjectPtr<UCanvasPanelSlot> PlayerMarkerSlot;
};
