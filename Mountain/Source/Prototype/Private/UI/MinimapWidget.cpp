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

void UMinimapWidget::SetPlayerMarkerUV(FVector2D UV)
{
    if (!PlayerMarker)
    {
        return;
    }

    // UV(0~1) → Canvas 픽셀 좌표 변환
    const FVector2D MapSize = GetMinimapImageSize();
    const FVector2D MarkerSize = PlayerMarker->GetDesiredSize();

    // 마커 중심이 UV 위치에 오도록 오프셋 적용
    const FVector2D PixelPos = UV * MapSize - MarkerSize * 0.5f;

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
    return FVector2D(512.f, 512.f);
}
