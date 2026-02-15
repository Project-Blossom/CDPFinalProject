#include "VoxelDensityGenerator.h"
#include "Math/UnrealMathUtility.h"

static FORCEINLINE float SmoothStep01(float t)
{
    t = FMath::Clamp(t, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

static FORCEINLINE float SmoothStep(float a, float b, float x)
{
    const float den = (b - a);
    if (FMath::IsNearlyZero(den)) return (x >= b) ? 1.f : 0.f;
    return SmoothStep01((x - a) / den);
}

static FORCEINLINE float SdBox(const FVector& p, const FVector& b)
{
    const FVector q(FMath::Abs(p.X), FMath::Abs(p.Y), FMath::Abs(p.Z));
    const FVector d = q - b;

    const float outside = FVector(
        FMath::Max(d.X, 0.f),
        FMath::Max(d.Y, 0.f),
        FMath::Max(d.Z, 0.f)
    ).Size();

    const float inside = FMath::Min(FMath::Max(d.X, FMath::Max(d.Y, d.Z)), 0.f);
    return outside + inside;
}

float FVoxelDensityGenerator::ComputeFrontInfluenceCm() const
{
    const float Warp = FMath::Max(0.f, S.WarpStrength) * FMath::Max(0.f, S.WarpAmpCm);

    const float Surface = FMath::Max(0.f, S.CliffSurfaceAmpCm);

    const float Base3D = (S.BaseField3DStrengthCm > 0.f)
        ? FMath::Clamp(S.BaseField3DStrengthCm, 0.f, 9000.f)
        : 0.f;

    const float Over = (S.VolumeStrength > 0.f)
        ? (FMath::Max(0.f, S.VolumeStrength) * FMath::Max(0.f, S.OverhangDepthCm))
        : 0.f;

    return (Surface + Base3D + Over + Warp);
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPosCm) const
{
    const float Iso = S.IsoLevel;

    const FVector Local = WorldPosCm - TerrainOriginWorld;
    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    // -------------------------------------------------
    // 1) 절벽 기본 파라미터
    // -------------------------------------------------
    const float CliffH = FMath::Max(1000.f, S.CliffHeightCm);
    const float FrontX = FMath::Max(200.f, S.CliffThicknessCm);

    const float z01 = Clamp01(
        (Local.Z - S.BaseHeightCm) / CliffH
    );

    // -------------------------------------------------
    // 2) 절벽 기본 면
    // -------------------------------------------------
    float density = (FrontX - Local.X);

    // -------------------------------------------------
    // 3) 과장된 Z-오버행 프로파일
    // -------------------------------------------------
    {
        const float curve = FMath::Pow(z01, 1.8f);

        const float OverhangAmp =
            FMath::Clamp(S.OverhangDepthCm, 500.f, 8000.f) *
            FMath::Clamp(S.VolumeStrength, 0.f, 2.0f);

        density += curve * OverhangAmp;
    }

    // -------------------------------------------------
    // 4) 실루엣용 메가 노이즈
    // -------------------------------------------------
    {
        const float Scale = FMath::Max(6000.f, S.DetailScaleCm * 3.f);
        const float n = FBM3D(SeededDomain(Local) / Scale, 3, 2.0f, 0.5f);

        const float Amp = FMath::Clamp(
            S.BaseField3DStrengthCm, 1500.f, 12000.f
        );

        density += n * Amp;
    }

    // -------------------------------------------------
    // 5) 표면 거칠기
    // -------------------------------------------------
    {
        const float Band = FMath::Max(150.f, Voxel * 3.f);
        const float surfMask =
            1.f - SmoothStep(0.f, Band, FMath::Abs(density - Iso));

        if (surfMask > 0.f)
        {
            const float Scale = FMath::Max(300.f, S.CliffSurfaceScaleCm);
            const float n = FBM3D(SeededDomain(Local) / Scale, 4, 2.0f, 0.55f);

            density +=
                n *
                FMath::Clamp(S.CliffSurfaceAmpCm, 0.f, 5000.f) *
                surfMask;
        }
    }

    // -------------------------------------------------
    // 6) 동굴
    // -------------------------------------------------
    if (S.CaveStrength > 0.f)
    {
        const float insideDepth = (FrontX - Local.X);

        if (insideDepth > FMath::Max(800.f, S.CaveNearSurfaceCm))
        {
            const float Scale = FMath::Max(600.f, S.CaveScaleCm);
            const float c =
                RidgedFBM01(SeededDomain(Local) / Scale, 4, 2.0f, 0.55f);

            const float th = FMath::Clamp(S.CaveThreshold, 0.4f, 0.75f);
            const float band = FMath::Clamp(S.CaveBand, 0.05f, 0.25f);

            const float hole = SmoothStep(th - band, th + band, c);

            density -=
                hole *
                FMath::Clamp(S.CaveDepthCm, 800.f, 8000.f) *
                FMath::Clamp(S.CaveStrength, 0.f, 2.5f);
        }
    }

    return density;
}
