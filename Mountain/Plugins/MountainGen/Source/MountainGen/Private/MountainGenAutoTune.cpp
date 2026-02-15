#include "MountainGenAutoTune.h"
#include "VoxelDensityGenerator.h"

#include "Math/UnrealMathUtility.h"

static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }
static FORCEINLINE bool InRange(float v, float mn, float mx) { return (v >= mn && v <= mx); }
static FORCEINLINE float Center(float mn, float mx) { return 0.5f * (mn + mx); }

static FVector MG_EstimateNormalFromDensity(const FVoxelDensityGenerator& Gen, const FVector& Pcm, float StepCm)
{
    const float dx = Gen.SampleDensity(Pcm + FVector(StepCm, 0, 0)) - Gen.SampleDensity(Pcm - FVector(StepCm, 0, 0));
    const float dy = Gen.SampleDensity(Pcm + FVector(0, StepCm, 0)) - Gen.SampleDensity(Pcm - FVector(0, StepCm, 0));
    const float dz = Gen.SampleDensity(Pcm + FVector(0, 0, StepCm)) - Gen.SampleDensity(Pcm - FVector(0, 0, StepCm));

    return (-FVector(dx, dy, dz)).GetSafeNormal();
}

static bool MG_FindFrontSurfacePoint_AlongX(
    const FVoxelDensityGenerator& Gen,
    float Iso,
    float XStart,
    float XEnd,
    float StepCm,
    int32 RefineIters,
    float Y,
    float Z,
    FVector& OutP)
{
    float x0 = XStart;
    float f0 = Gen.SampleDensity(FVector(x0, Y, Z)) - Iso;

    for (float x1 = x0 - StepCm; x1 >= XEnd; x1 -= StepCm)
    {
        const float f1 = Gen.SampleDensity(FVector(x1, Y, Z)) - Iso;

        if ((f0 >= 0.f && f1 < 0.f) || (f0 < 0.f && f1 >= 0.f))
        {
            float a = x1, b = x0;
            float fa = f1, fb = f0;

            RefineIters = FMath::Clamp(RefineIters, 1, 12);
            for (int32 i = 0; i < RefineIters; ++i)
            {
                const float m = 0.5f * (a + b);
                const float fm = Gen.SampleDensity(FVector(m, Y, Z)) - Iso;

                if ((fa >= 0.f && fm < 0.f) || (fa < 0.f && fm >= 0.f))
                {
                    b = m; fb = fm;
                }
                else
                {
                    a = m; fa = fm;
                }
            }

            OutP = FVector(0.5f * (a + b), Y, Z);
            return true;
        }

        x0 = x1;
        f0 = f1;
    }

    return false;
}

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    FMGMetrics M;
    const float Iso = S.IsoLevel;

    M.CaveSamples = 0;
    M.CaveVoidRatio = 0.f;

    if (!S.bUseCliffBase)
    {
        M.SurfaceNearSamples = 0;
        M.OverhangRatio = 0.f;
        M.SteepRatio = 0.f;
        return M;
    }

    FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    const float HalfW = FMath::Max(1.f, S.CliffHalfWidthCm);
    const float H = FMath::Max(1.f, S.CliffHeightCm);
    const float FrontX = S.CliffThicknessCm;

    const float z0 = S.BaseHeightCm;
    const float z1 = S.BaseHeightCm + H;

    const float Influence = Gen.ComputeFrontInfluenceCm();
    const float NoiseMinDisp = (Influence > 0.f) ? (0.15f * Influence) : 0.f;

    const float Pad = FMath::Max(20.f, Voxel * 2.f);
    const float XStart = FrontX + Influence + Pad;
    const float XEnd = FrontX - (Influence + Pad);

    float GridStep = (S.MetricsStepCm > 0.f) ? S.MetricsStepCm : FMath::Max(20.f, Voxel * 2.0f);

    const float Area = (2.f * HalfW) * (H);
    const int32 MaxSamplesBudget = 8192;
    {
        const float approx = Area / FMath::Max(1.f, GridStep * GridStep);
        if (approx > (float)MaxSamplesBudget)
        {
            const float k = FMath::Sqrt(approx / (float)MaxSamplesBudget);
            GridStep *= k;
        }
    }

    const float ScanStep = FMath::Max(Voxel * 1.0f, GridStep * 0.5f);
    const int32 RefineIters = 7;
    const float NormalStep = FMath::Max(40.f, Voxel * 2.0f);

    const float SteepDotThreshold = S.SteepDotThreshold;

    int32 Valid = 0, Over = 0, Steep = 0;

    for (float Z = z0 + GridStep * 0.5f; Z <= z1 - GridStep * 0.5f; Z += GridStep)
    {
        for (float Y = -HalfW + GridStep * 0.5f; Y <= HalfW - GridStep * 0.5f; Y += GridStep)
        {
            FVector P;
            if (!MG_FindFrontSurfacePoint_AlongX(Gen, Iso, XStart, XEnd, ScanStep, RefineIters, Y, Z, P))
                continue;

            const float Disp = FMath::Abs(P.X - FrontX);
            if (NoiseMinDisp > 0.f && Disp < NoiseMinDisp)
                continue;

            FVector N = MG_EstimateNormalFromDensity(Gen, P, NormalStep);

            if (FVector::DotProduct(N, FVector(1, 0, 0)) < 0.f)
                N = -N;

            const float UpDot = FVector::DotProduct(N, FVector::UpVector);

            Valid++;
            if (UpDot < 0.f) Over++;
            if (FMath::Abs(UpDot) <= SteepDotThreshold) Steep++;
        }
    }

    M.SurfaceNearSamples = Valid;
    M.OverhangRatio = (Valid > 0) ? (float)Over / (float)Valid : 0.f;
    M.SteepRatio = (Valid > 0) ? (float)Steep / (float)Valid : 0.f;

    return M;
}

// -------------------- Feedback steps --------------------
static void Feedback_Overhang(FMountainGenSettings& S, float tgtMin, float tgtMax, float measured)
{
    const float err = Center(tgtMin, tgtMax) - measured;

    S.OverhangBias = Clamp01(S.OverhangBias - err * 0.30f);
    S.OverhangDepthCm = FMath::Clamp(S.OverhangDepthCm + err * 1400.f, 200.f, 15000.f);
    S.VolumeStrength = FMath::Clamp(S.VolumeStrength + err * 0.22f, 0.f, 3.f);
    S.WarpStrength = FMath::Clamp(S.WarpStrength + err * 0.15f, 0.f, 3.f);
}

static void Feedback_Steep(FMountainGenSettings& S, float tgtMin, float tgtMax, float measured)
{
    const float err = Center(tgtMin, tgtMax) - measured;

    S.BaseField3DStrengthCm = FMath::Clamp(S.BaseField3DStrengthCm + err * 13000.f, 2000.f, 30000.f);
    S.DetailScaleCm = FMath::Clamp(S.DetailScaleCm - err * 1200.f, 300.f, 20000.f);
    S.WarpStrength = FMath::Clamp(S.WarpStrength + err * 0.10f, 0.f, 3.f);
}

bool MGTuneSettingsFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    const int32 MaxIters = FMath::Clamp(InOutS.AutoTuneMaxIters, 1, 20);

    const float FixedHeightAmp = InOutS.HeightAmpCm;
    const float FixedRadius = InOutS.EnvelopeRadiusCm;
    const float FixedBaseH = InOutS.BaseHeightCm;

    for (int32 iter = 0; iter < MaxIters; ++iter)
    {
        InOutS.HeightAmpCm = FixedHeightAmp;
        InOutS.EnvelopeRadiusCm = FixedRadius;
        InOutS.BaseHeightCm = FixedBaseH;

        const FMGMetrics M = MGComputeMetricsQuick(InOutS, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        const bool bOverOK = InRange(M.OverhangRatio, InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
        const bool bSteepOK = InRange(M.SteepRatio, InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);

        if (bOverOK && bSteepOK)
            return true;

        if (!bOverOK)  Feedback_Overhang(InOutS, InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax, M.OverhangRatio);
        if (!bSteepOK) Feedback_Steep(InOutS, InOutS.Targets.SteepMin, InOutS.Targets.SteepMax, M.SteepRatio);

        InOutS.WarpStrength = FMath::Clamp(InOutS.WarpStrength, 0.f, 3.f);
        InOutS.VolumeStrength = FMath::Clamp(InOutS.VolumeStrength, 0.f, 3.f);

        InOutS.BaseField3DScaleCm = FMath::Max(InOutS.BaseField3DScaleCm, 1000.f);
        InOutS.DetailScaleCm = FMath::Clamp(InOutS.DetailScaleCm, 300.f, InOutS.BaseField3DScaleCm);
    }

    InOutS.HeightAmpCm = FixedHeightAmp;
    InOutS.EnvelopeRadiusCm = FixedRadius;
    InOutS.BaseHeightCm = FixedBaseH;

    return false;
}

// ============================================================
// Seed
// ============================================================
void MGDeriveParamsFromSeed(FMountainGenSettings& InOutS, int32 Seed)
{
    if (Seed <= 0) Seed = 1557;
    InOutS.Seed = Seed;

    FRandomStream Rng(Seed ^ 0x4C1D7A2B);

    // Base 3D field
    InOutS.BaseField3DStrengthCm = Rng.FRandRange(9000.f, 22000.f);
    InOutS.BaseField3DScaleCm = Rng.FRandRange(12000.f, 42000.f);

    // Detail
    InOutS.DetailScaleCm = Rng.FRandRange(5000.f, 18000.f);

    // Warp
    InOutS.WarpStrength = Rng.FRandRange(0.40f, 1.05f);
    InOutS.WarpAmpCm = Rng.FRandRange(0.f, 4200.f);
    InOutS.WarpPatchCm = Rng.FRandRange(8000.f, 28000.f);

    // Overhang
    InOutS.VolumeStrength = Rng.FRandRange(0.35f, 1.25f);
    InOutS.OverhangScaleCm = Rng.FRandRange(3500.f, 14000.f);
    InOutS.OverhangBias = Rng.FRandRange(0.52f, 0.70f);
    InOutS.OverhangDepthCm = Rng.FRandRange(800.f, 5200.f);
}

bool MGFinalizeSettingsFromSeed(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    if (!InOutS.bAutoTune)
        return true;

    return MGTuneSettingsFeedback(InOutS, TerrainOriginWorld, WorldMinCm, WorldMaxCm);
}

bool MGIsSatisfiedToTargets(const FMountainGenSettings& S, const FMGMetrics& M)
{
    const bool bOverOK = InRange(M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax);
    const bool bSteepOK = InRange(M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax);
    return (bOverOK && bSteepOK);
}

FMGMetrics MGComputeMetricsFullGrid(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    return MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMinCm, WorldMaxCm);
}