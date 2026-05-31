// File: Source/Prototype/Private/UI/MinimapWidget.cpp
#include "UI/MinimapWidget.h"

#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/RetainerBox.h"

void UMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();

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

    FSlateBrush Brush = MinimapImage->GetBrush();
    Brush.SetUVRegion(FBox2D(UVOffset, UVOffset + UVScale));
    MinimapImage->SetBrush(Brush);
}

void UMinimapWidget::SetPlayerMarkerPosition(FVector2D NormalizedPos)
{
    if (!PlayerMarker || !PlayerMarkerSlot)
    {
        return;
    }

    // RetainerBox가 있으면 그 Canvas Panel Slot 기준, 없으면 MinimapImage Slot 기준
    UWidget* ReferenceWidget = MinimapRetainer
        ? StaticCast<UWidget*>(MinimapRetainer)
        : StaticCast<UWidget*>(MinimapImage);

    if (!ReferenceWidget)
    {
        return;
    }

    UCanvasPanelSlot* RefSlot = Cast<UCanvasPanelSlot>(ReferenceWidget->Slot);
    if (!RefSlot)
    {
        return;
    }

    const FVector2D ImagePos   = RefSlot->GetPosition();
    const FVector2D ImageSize  = RefSlot->GetSize();
    const FVector2D ImageAlign = RefSlot->GetAlignment();
    const FVector2D MarkerSize = FVector2D(16.f, 16.f);

    // Anchor 기준 상대좌표에서 좌상단 계산
    const FVector2D ImageTopLeft = ImagePos - ImageSize * ImageAlign;

    // 마커 중심이 NormalizedPos에 오도록 계산
    const FVector2D MarkerPos = ImageTopLeft + NormalizedPos * ImageSize - MarkerSize * 0.5f;

    PlayerMarkerSlot->SetPosition(MarkerPos);
}

FVector2D UMinimapWidget::GetMinimapImageSize() const
{
    if (MinimapRetainer)
    {
        UCanvasPanelSlot* RetainerSlot = Cast<UCanvasPanelSlot>(MinimapRetainer->Slot);
        if (RetainerSlot)
        {
            return RetainerSlot->GetSize();
        }
    }
    if (MinimapImage)
    {
        const FSlateBrush& Brush = MinimapImage->GetBrush();
        const FVector2D BrushSize = Brush.ImageSize;
        if (!BrushSize.IsNearlyZero())
        {
            return BrushSize;
        }
        return MinimapImage->GetDesiredSize();
    }
    return FVector2D(512.f, 512.f);
}
