#include "MountainGenAutoTune.h"
#include "VoxelDensityGenerator.h"

struct FMGRange
{
    float Min = 0.f;
    float Max = 0.f;

    bool Contains(float v) const { return v >= Min && v <= Max; }
    float Center() const { return 0.5f * (Min + Max); }
};

static FORCEINLINE float MGClamp01(float x)
{
    return FMath::Clamp(x, 0.f, 1.f);
}

static FVector MGEstimateNormalFromDensity(
    const FVoxelDensityGenerator& Gen,
    const FVector& Pcm,
    float StepCm)
{
    const float dx = Gen.SampleDensity(Pcm + FVector(StepCm, 0, 0)) - Gen.SampleDensity(Pcm - FVector(StepCm, 0, 0));
    const float dy = Gen.SampleDensity(Pcm + FVector(0, StepCm, 0)) - Gen.SampleDensity(Pcm - FVector(0, StepCm, 0));
    const float dz = Gen.SampleDensity(Pcm + FVector(0, 0, StepCm)) - Gen.SampleDensity(Pcm - FVector(0, 0, StepCm));
    return FVector(dx, dy, dz).GetSafeNormal();
}

static FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    FMGMetrics M;
    const float Iso = S.IsoLevel;

    FRandomStream Rng((int32)(S.Seed ^ 0x51A3B9D1));
    FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

    // ----------------------------
    // (1) CaveVoidRatio
    // ----------------------------
    const int32 CaveSamples = 8000;
    int32 Air = 0;

    float Z0 = WorldMinCm.Z + S.CaveMinHeightCm;
    float Z1 = WorldMinCm.Z + S.CaveMaxHeightCm;
    Z0 = FMath::Clamp(Z0, WorldMinCm.Z, WorldMaxCm.Z);
    Z1 = FMath::Clamp(Z1, WorldMinCm.Z, WorldMaxCm.Z);

    for (int32 i = 0; i < CaveSamples; ++i)
    {
        const float x = Rng.FRandRange(WorldMinCm.X, WorldMaxCm.X);
        const float y = Rng.FRandRange(WorldMinCm.Y, WorldMaxCm.Y);
        const float z = Rng.FRandRange(Z0, Z1);

        const float d = Gen.SampleDensity(FVector(x, y, z));
        if (d < Iso) Air++;
    }

    M.CaveSamples = CaveSamples;
    M.CaveVoidRatio = (CaveSamples > 0) ? (float)Air / (float)CaveSamples : 0.f;

    // ----------------------------
    // (2) OverhangRatio / SteepRatio
    // ----------------------------
    const int32 SurfaceTry = 45000;
    const float SurfaceEps = 25.f;
    const float NormalStep = FMath::Max(2.f * S.VoxelSizeCm, 80.f);

    const float SteepThreshold = 0.60f;

    int32 Near = 0;
    int32 Over = 0;
    int32 Steep = 0;

    for (int32 i = 0; i < SurfaceTry; ++i)
    {
        const float x = Rng.FRandRange(WorldMinCm.X, WorldMaxCm.X);
        const float y = Rng.FRandRange(WorldMinCm.Y, WorldMaxCm.Y);
        const float z = Rng.FRandRange(WorldMinCm.Z, WorldMaxCm.Z);

        const FVector P(x, y, z);
        const float d = Gen.SampleDensity(P);

        if (FMath::Abs(d - Iso) > SurfaceEps)
            continue;

        const FVector N = MGEstimateNormalFromDensity(Gen, P, NormalStep);
        const float UpDot = FVector::DotProduct(N, FVector::UpVector);

        Near++;

        if (UpDot < 0.0f) Over++;

        const float Steepness = 1.0f - FMath::Abs(UpDot);
        if (Steepness >= SteepThreshold) Steep++;
    }

    M.SurfaceNearSamples = Near;
    M.OverhangRatio = (Near > 0) ? (float)Over / (float)Near : 0.f;
    M.SteepRatio = (Near > 0) ? (float)Steep / (float)Near : 0.f;

    return M;
}

static void MGFeedbackStep_Cave(FMountainGenSettings& S, const FMGRange& Target, float MeasuredVoid)
{
    const float Err = Target.Center() - MeasuredVoid;

    const float GainTh = 0.35f;
    S.CaveThreshold = MGClamp01(S.CaveThreshold - Err * GainTh);

    const float GainStr = 0.25f;
    S.CaveStrength = FMath::Clamp(S.CaveStrength + Err * GainStr, 0.f, 3.f);

    const float GainBand = 0.15f;
    S.CaveBand = MGClamp01(S.CaveBand + Err * GainBand);
}

static void MGFeedbackStep_Overhang(FMountainGenSettings& S, const FMGRange& Target, float MeasuredOver)
{
    const float Err = Target.Center() - MeasuredOver;

    const float GainBias = 0.30f;
    S.OverhangBias = MGClamp01(S.OverhangBias - Err * GainBias);

    const float GainDepth = 1400.f;
    S.OverhangDepthCm = FMath::Clamp(S.OverhangDepthCm + Err * GainDepth, 200.f, 15000.f);

    const float GainVol = 0.22f;
    S.VolumeStrength = FMath::Clamp(S.VolumeStrength + Err * GainVol, 0.f, 3.f);

    const float GainWarp = 0.15f;
    S.WarpStrength = FMath::Clamp(S.WarpStrength + Err * GainWarp, 0.f, 3.f);
}

static void MGFeedbackStep_Steep(FMountainGenSettings& S, const FMGRange& Target, float MeasuredSteep)
{
    const float Err = Target.Center() - MeasuredSteep;

    const float Gain = 0.55f;
    const float Factor = FMath::Clamp(FMath::Exp(-Err * Gain), 0.75f, 1.35f);
    S.DetailScaleCm *= Factor;

    const float WarpGain = 0.10f;
    S.WarpStrength = FMath::Clamp(S.WarpStrength + Err * WarpGain, 0.f, 3.f);
}

bool MGTuneSettingsFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    const FMGRange CaveT{ InOutS.Targets.CaveMin, InOutS.Targets.CaveMax };
    const FMGRange OverT{ InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax };
    const FMGRange SteepT{ InOutS.Targets.SteepMin, InOutS.Targets.SteepMax };

    const int32 MaxIters = FMath::Clamp(InOutS.AutoTuneMaxIters, 1, 20);

    for (int32 Iter = 0; Iter < MaxIters; ++Iter)
    {
        const FMGMetrics M = MGComputeMetricsQuick(InOutS, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        const bool bCaveOK = CaveT.Contains(M.CaveVoidRatio);
        const bool bOverOK = OverT.Contains(M.OverhangRatio);
        const bool bSteepOK = SteepT.Contains(M.SteepRatio);

        if (bCaveOK && bOverOK && bSteepOK)
            return true;

        if (!bCaveOK) MGFeedbackStep_Cave(InOutS, CaveT, M.CaveVoidRatio);
        if (!bOverOK) MGFeedbackStep_Overhang(InOutS, OverT, M.OverhangRatio);
        if (!bSteepOK) MGFeedbackStep_Steep(InOutS, SteepT, M.SteepRatio);

        // safety clamps
        InOutS.WarpStrength = FMath::Clamp(InOutS.WarpStrength, 0.f, 3.f);
        InOutS.VolumeStrength = FMath::Clamp(InOutS.VolumeStrength, 0.f, 3.f);
        InOutS.CaveStrength = FMath::Clamp(InOutS.CaveStrength, 0.f, 3.f);

        InOutS.CaveScaleCm = FMath::Max(InOutS.CaveScaleCm, 100.f);
        InOutS.OverhangFadeCm = FMath::Clamp(InOutS.OverhangFadeCm, 50.f, 60000.f);

        InOutS.WorldScaleCm = FMath::Max(InOutS.WorldScaleCm, 1000.f);
        InOutS.DetailScaleCm = FMath::Clamp(InOutS.DetailScaleCm, 300.f, InOutS.WorldScaleCm);
    }

    return false;
}