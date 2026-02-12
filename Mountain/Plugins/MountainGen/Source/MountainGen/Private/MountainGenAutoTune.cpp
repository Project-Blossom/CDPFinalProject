#include "MountainGenAutoTune.h"

#include "Math/UnrealMathUtility.h"
#include "Math/RandomStream.h"

static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }
static FORCEINLINE bool InRange(float v, float mn, float mx) { return (v >= mn && v <= mx); }
static FORCEINLINE float Center(float mn, float mx) { return 0.5f * (mn + mx); }

static FVector MG_EstimateNormalFromDensity(const FVoxelDensityGenerator& Gen, const FVector& Pcm, float StepCm)
{
    const float dx = Gen.SampleDensity(Pcm + FVector(StepCm, 0, 0)) - Gen.SampleDensity(Pcm - FVector(StepCm, 0, 0));
    const float dy = Gen.SampleDensity(Pcm + FVector(0, StepCm, 0)) - Gen.SampleDensity(Pcm - FVector(0, StepCm, 0));
    const float dz = Gen.SampleDensity(Pcm + FVector(0, 0, StepCm)) - Gen.SampleDensity(Pcm - FVector(0, 0, StepCm));
    return FVector(dx, dy, dz).GetSafeNormal();
}

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    FMGMetrics M;
    const float Iso = S.IsoLevel;

    // Seed 고정 랜덤
    FRandomStream Rng((int32)(S.Seed ^ 0x51A3B9D1));
    FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

    // ------------------------------------------------------------
    // (1) CaveVoidRatio
    // ------------------------------------------------------------
    M.CaveSamples = 0;
    M.CaveVoidRatio = 0.f;

    // ------------------------------------------------------------
    // (2) OverhangRatio / SteepRatio
    // ------------------------------------------------------------
    const int32 SurfaceTry = 60000;
    const float SurfaceEps = FMath::Max(10.f, S.VoxelSizeCm * 0.75f);
    const float NormalStep = FMath::Max(2.f * S.VoxelSizeCm, 40.f);
    const float SteepDotThreshold = S.SteepDotThreshold;

    int32 Near = 0, Over = 0, Steep = 0;

    for (int32 i = 0; i < SurfaceTry; ++i)
    {
        const FVector P(
            Rng.FRandRange(WorldMinCm.X, WorldMaxCm.X),
            Rng.FRandRange(WorldMinCm.Y, WorldMaxCm.Y),
            Rng.FRandRange(WorldMinCm.Z, WorldMaxCm.Z)
        );

        const float d = Gen.SampleDensity(P);
        if (FMath::Abs(d - Iso) > SurfaceEps)
            continue;

        const FVector N = MG_EstimateNormalFromDensity(Gen, P, NormalStep);
        const float UpDot = FVector::DotProduct(N, FVector::UpVector);

        Near++;
        if (UpDot < 0.f) Over++;
        if (FMath::Abs(UpDot) <= SteepDotThreshold) Steep++;
    }

    M.SurfaceNearSamples = Near;
    M.OverhangRatio = (Near > 0) ? (float)Over / (float)Near : 0.f;
    M.SteepRatio = (Near > 0) ? (float)Steep / (float)Near : 0.f;

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

bool MGFindFinalSeedByFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm,
    int32 StartSeed,
    int32 Tries)
{
    if (Tries <= 0) Tries = 1;

    const float FixedHeightAmp = InOutS.HeightAmpCm;
    const float FixedRadius = InOutS.EnvelopeRadiusCm;
    const float FixedBaseH = InOutS.BaseHeightCm;

    FRandomStream Rng(StartSeed ^ 0x17C0E9B5);

    float BestScore = FLT_MAX;
    int32 BestSeed = StartSeed;
    FMountainGenSettings BestS = InOutS;

    auto ScoreToRange = [](float v, float mn, float mx)
        {
            if (v < mn) return (mn - v);
            if (v > mx) return (v - mx);
            return 0.f;
        };

    for (int32 i = 0; i < Tries; ++i)
    {
        const int32 Seed = (i == 0) ? StartSeed : Rng.RandRange(1, INT32_MAX);

        FMountainGenSettings Cand = InOutS;
        Cand.Seed = Seed;

        MGDeriveParamsFromSeed(Cand, Seed);
        (void)MGFinalizeSettingsFromSeed(Cand, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        Cand.HeightAmpCm = FixedHeightAmp;
        Cand.EnvelopeRadiusCm = FixedRadius;
        Cand.BaseHeightCm = FixedBaseH;

        const FMGMetrics M = MGComputeMetricsQuick(Cand, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        const float Score =
            ScoreToRange(M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax) +
            ScoreToRange(M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestSeed = Seed;
            BestS = Cand;
        }

        if (MGIsSatisfiedToTargets(Cand, M))
        {
            InOutS = Cand;
            return true;
        }
    }

    InOutS = BestS;
    InOutS.Seed = BestSeed;

    InOutS.HeightAmpCm = FixedHeightAmp;
    InOutS.EnvelopeRadiusCm = FixedRadius;
    InOutS.BaseHeightCm = FixedBaseH;

    return false;
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