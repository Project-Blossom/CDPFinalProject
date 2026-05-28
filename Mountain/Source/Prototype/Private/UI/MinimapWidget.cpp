// File: Source/Prototype/Private/UI/MinimapWidget.cpp
#include "UI/MinimapWidget.h"

#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // PlayerMarker의 CanvasPanelSlot 캐시 (SetPosition 효율화)
    if (PlayerMarker)
    {
        PlayerMarkerSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot);
    }
}

void UMinimapWidget::SetMinimapTint(const FLinearColor& Tint)
{
    if (MinimapImage)
    {
        MinimapImage->SetColorAndOpacity(Tint);
    }
}

void UMinimapWidget::SetMinimapUVRegion(FVector2D UVOffset, FVector2D UVScale)
{
    if (!MinimapImage)
    {
        return;
    }

    // FSlateBrush의 UVRegion을 수정하여 RT_Minimap의 일부 영역만 표시
    // UVOffset: 표시 시작점, UVScale: 표시 범위 크기
    FSlateBrush Brush = MinimapImage->GetBrush();
    Brush.SetUVRegion(FBox2D(UVOffset, UVOffset + UVScale));
    MinimapImage->SetBrush(Brush);
}

void UMinimapWidget::SetPlayerMarkerPosition(FVector2D NormalizedPos)
{
    if (!PlayerMarker)
    {
        return;
    }

    // NormalizedPos (0~1) → 미니맵 위젯 픽셀 좌표 변환
    // 일반 상태: NormalizedPos = (0.5, 0.5) → 마커가 미니맵 중앙에 고정
    const FVector2D MapSize    = GetMinimapImageSize();
    const FVector2D MarkerSize = PlayerMarker->GetDesiredSize();
    const FVector2D PixelPos   = NormalizedPos * MapSize - MarkerSize * 0.5f;

    if (PlayerMarkerSlot)
    {
        PlayerMarkerSlot->SetPosition(PixelPos);
    }
}

FVector2D UMinimapWidget::GetMinimapImageSize() const
{
    if (MinimapImage)
    {
        return MinimapImage->GetDesiredSize();
    }
    return FVector2D(256.f, 256.f);
}
