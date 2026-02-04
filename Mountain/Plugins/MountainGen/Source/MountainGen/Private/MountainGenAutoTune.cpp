#include "MountainGenAutoTune.h"

#include "Math/UnrealMathUtility.h"
#include "Math/RandomStream.h"

static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }
static FORCEINLINE bool InRange(float v, float mn, float mx) { return (v >= mn && v <= mx); }
static FORCEINLINE float Center(float mn, float mx) { return 0.5f * (mn + mx); }

static FVector EstimateNormalFromDensity(const FVoxelDensityGenerator& Gen, const FVector& Pcm, float StepCm)
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
    // (1) CaveVoidRatio : 지정 높이 밴드에서 "공기 비율"
    // ------------------------------------------------------------
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

    // ------------------------------------------------------------
    // (2) OverhangRatio / SteepRatio : 표면 근처 샘플에서 노멀로 판단
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

        const FVector N = EstimateNormalFromDensity(Gen, P, NormalStep);
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
static void Feedback_Cave(FMountainGenSettings& S, float tgtMin, float tgtMax, float measured)
{
    const float err = Center(tgtMin, tgtMax) - measured;

    S.CaveThreshold = Clamp01(S.CaveThreshold - err * 0.35f);
    S.CaveStrength = FMath::Clamp(S.CaveStrength + err * 0.25f, 0.f, 3.f);
    S.CaveBand = Clamp01(S.CaveBand + err * 0.15f);
}

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

    const float factor = FMath::Clamp(FMath::Exp(-err * 0.55f), 0.75f, 1.35f);
    S.DetailScaleCm *= factor;

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

        const bool bCaveOK = InRange(M.CaveVoidRatio, InOutS.Targets.CaveMin, InOutS.Targets.CaveMax);
        const bool bOverOK = InRange(M.OverhangRatio, InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
        const bool bSteepOK = InRange(M.SteepRatio, InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);

        if (bCaveOK && bOverOK && bSteepOK)
            return true;

        if (!bCaveOK)  Feedback_Cave(InOutS, InOutS.Targets.CaveMin, InOutS.Targets.CaveMax, M.CaveVoidRatio);
        if (!bOverOK)  Feedback_Overhang(InOutS, InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax, M.OverhangRatio);
        if (!bSteepOK) Feedback_Steep(InOutS, InOutS.Targets.SteepMin, InOutS.Targets.SteepMax, M.SteepRatio);

        InOutS.WarpStrength = FMath::Clamp(InOutS.WarpStrength, 0.f, 3.f);
        InOutS.VolumeStrength = FMath::Clamp(InOutS.VolumeStrength, 0.f, 3.f);
        InOutS.CaveStrength = FMath::Clamp(InOutS.CaveStrength, 0.f, 3.f);

        InOutS.CaveScaleCm = FMath::Max(InOutS.CaveScaleCm, 100.f);
        InOutS.BaseField3DScaleCm = FMath::Max(InOutS.BaseField3DScaleCm, 1000.f);
        InOutS.DetailScaleCm = FMath::Clamp(InOutS.DetailScaleCm, 300.f, InOutS.BaseField3DScaleCm);
    }

    InOutS.HeightAmpCm = FixedHeightAmp;
    InOutS.EnvelopeRadiusCm = FixedRadius;
    InOutS.BaseHeightCm = FixedBaseH;

    return false;
}

// ============================================================
// Seed -> 파라미터 파생
// ============================================================
void MGDeriveParamsFromSeed(FMountainGenSettings& InOutS, int32 Seed)
{
    if (Seed <= 0) Seed = 1557;
    InOutS.Seed = Seed;

    FRandomStream Rng(Seed ^ 0x4C1D7A2B);

    // Base 3D field
    InOutS.BaseField3DStrengthCm = Rng.FRandRange(9000.f, 22000.f);
    InOutS.BaseField3DScaleCm = Rng.FRandRange(9000.f, 26000.f);
    InOutS.BaseField3DOctaves = Rng.RandRange(3, 6);
    InOutS.BaseField3DRidgedPower = Rng.FRandRange(1.6f, 2.6f);

    // Detail
    InOutS.DetailScaleCm = Rng.FRandRange(1800.f, 9000.f);

    // Warp
    InOutS.WarpStrength = Rng.FRandRange(0.6f, 2.2f);
    InOutS.WarpPatchCm = Rng.FRandRange(9000.f, 24000.f);
    InOutS.WarpAmpCm = Rng.FRandRange(2500.f, 9000.f);

    // Overhang
    InOutS.VolumeStrength = Rng.FRandRange(0.6f, 2.2f);
    InOutS.OverhangScaleCm = Rng.FRandRange(4500.f, 14000.f);
    InOutS.OverhangBias = Rng.FRandRange(0.45f, 0.70f);
    InOutS.OverhangDepthCm = Rng.FRandRange(800.f, 7000.f);
    InOutS.OverhangFadeCm = Rng.FRandRange(8000.f, 26000.f);

    // Caves
    InOutS.CaveStrength = Rng.FRandRange(0.4f, 2.2f);
    InOutS.CaveScaleCm = Rng.FRandRange(4500.f, 14000.f);
    InOutS.CaveThreshold = Rng.FRandRange(0.52f, 0.74f);
    InOutS.CaveBand = Rng.FRandRange(0.06f, 0.18f);
    InOutS.CaveDepthCm = Rng.FRandRange(600.f, 3500.f);

    // Gravity
    InOutS.GravityStrength = Rng.FRandRange(0.8f, 1.4f);
    InOutS.GravityScale = Rng.FRandRange(1.5f, 3.2f);

    // EnvelopeEdgeCut 최소 보정
    InOutS.EnvelopeEdgeFadeCm = FMath::Max(500.f, InOutS.EnvelopeEdgeFadeCm);
    InOutS.EnvelopeEdgeCutCm = FMath::Max(15000.f, InOutS.EnvelopeEdgeCutCm);
}

// ============================================================
// Seed에서 파생된 값 + 피드백 튠 마무리
// ============================================================
bool MGFinalizeSettingsFromSeed(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    const float FixedHeightAmp = InOutS.HeightAmpCm;
    const float FixedRadius = InOutS.EnvelopeRadiusCm;
    const float FixedBaseH = InOutS.BaseHeightCm;

    const bool bOK = MGTuneSettingsFeedback(InOutS, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

    InOutS.HeightAmpCm = FixedHeightAmp;
    InOutS.EnvelopeRadiusCm = FixedRadius;
    InOutS.BaseHeightCm = FixedBaseH;

    return bOK;
}

static float ScoreToTargets(const FMountainGenSettings& S, const FMGMetrics& M)
{
    const float cC = Center(S.Targets.CaveMin, S.Targets.CaveMax);
    const float cO = Center(S.Targets.OverhangMin, S.Targets.OverhangMax);
    const float cS = Center(S.Targets.SteepMin, S.Targets.SteepMax);

    return FMath::Abs(M.CaveVoidRatio - cC)
        + FMath::Abs(M.OverhangRatio - cO)
        + FMath::Abs(M.SteepRatio - cS);
}

// ============================================================
// Seed 탐색 후보 시드를 여러 개 보고, 목표에 가장 가까운 것을 선택
// ============================================================
bool MGFindFinalSeedByFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm,
    int32 StartSeed,
    int32 Tries)
{
    Tries = FMath::Max(1, Tries);
    if (StartSeed <= 0) StartSeed = 1337;

    const float FixedHeightAmp = InOutS.HeightAmpCm;
    const float FixedRadius = InOutS.EnvelopeRadiusCm;
    const float FixedBaseH = InOutS.BaseHeightCm;

    FRandomStream SeedRng(StartSeed ^ 0x19D3A7F1);

    float BestScore = BIG_NUMBER;
    FMountainGenSettings BestS = InOutS;
    int32 BestSeed = StartSeed;

    for (int32 i = 0; i < Tries; ++i)
    {
        const int32 CandidateSeed = SeedRng.RandRange(1, INT32_MAX);

        FMountainGenSettings Cand = InOutS;
        Cand.Seed = CandidateSeed;

        Cand.HeightAmpCm = FixedHeightAmp;
        Cand.EnvelopeRadiusCm = FixedRadius;
        Cand.BaseHeightCm = FixedBaseH;

        MGDeriveParamsFromSeed(Cand, CandidateSeed);
        (void)MGFinalizeSettingsFromSeed(Cand, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        const FMGMetrics M = MGComputeMetricsQuick(Cand, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        const bool bCaveOK = InRange(M.CaveVoidRatio, Cand.Targets.CaveMin, Cand.Targets.CaveMax);
        const bool bOverOK = InRange(M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax);
        const bool bSteepOK = InRange(M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax);

        const float Score = ScoreToTargets(Cand, M);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestS = Cand;
            BestSeed = CandidateSeed;
        }

        if (bCaveOK && bOverOK && bSteepOK)
        {
            InOutS = Cand;
            InOutS.Seed = CandidateSeed;

            // 고정값 복구
            InOutS.HeightAmpCm = FixedHeightAmp;
            InOutS.EnvelopeRadiusCm = FixedRadius;
            InOutS.BaseHeightCm = FixedBaseH;

            return true;
        }
    }

    // 못 찾으면 가장 가까운 후보로 확정
    InOutS = BestS;
    InOutS.Seed = BestSeed;

    InOutS.HeightAmpCm = FixedHeightAmp;
    InOutS.EnvelopeRadiusCm = FixedRadius;
    InOutS.BaseHeightCm = FixedBaseH;

    return false;
}

bool MGIsSatisfiedToTargets(const FMountainGenSettings& S, const FMGMetrics& M)
{
    const bool bCaveOK = InRange(M.CaveVoidRatio, S.Targets.CaveMin, S.Targets.CaveMax);
    const bool bOverOK = InRange(M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax);
    const bool bSteepOK = InRange(M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax);
    return (bCaveOK && bOverOK && bSteepOK);
}

FMGMetrics MGComputeMetricsFullGrid(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    FMGMetrics M;
    const float Iso = S.IsoLevel;

    FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

    const float StepCm = (S.MetricsStepCm > 0.0f)
        ? S.MetricsStepCm
        : FMath::Max(100.f, S.VoxelSizeCm * 2.0f);

    float Z0 = WorldMinCm.Z + S.CaveMinHeightCm;
    float Z1 = WorldMinCm.Z + S.CaveMaxHeightCm;
    Z0 = FMath::Clamp(Z0, WorldMinCm.Z, WorldMaxCm.Z);
    Z1 = FMath::Clamp(Z1, WorldMinCm.Z, WorldMaxCm.Z);

    const float SurfaceEps = FMath::Max(10.f, S.VoxelSizeCm * 0.75f);
    const float NormalStep = FMath::Max(2.f * S.VoxelSizeCm, 40.f);

    int32 CaveTotal = 0;
    int32 CaveAir = 0;

    int32 Near = 0;
    int32 Over = 0;
    int32 Steep = 0;

    for (float z = WorldMinCm.Z; z <= WorldMaxCm.Z + KINDA_SMALL_NUMBER; z += StepCm)
    {
        for (float y = WorldMinCm.Y; y <= WorldMaxCm.Y + KINDA_SMALL_NUMBER; y += StepCm)
        {
            for (float x = WorldMinCm.X; x <= WorldMaxCm.X + KINDA_SMALL_NUMBER; x += StepCm)
            {
                const FVector P(x, y, z);
                const float d = Gen.SampleDensity(P);

                // Cave void ratio
                if (z >= Z0 && z <= Z1)
                {
                    CaveTotal++;
                    if (d < Iso) CaveAir++;
                }

                // surface-derived
                if (FMath::Abs(d - Iso) <= SurfaceEps)
                {
                    const FVector N = EstimateNormalFromDensity(Gen, P, NormalStep);
                    const float UpDot = FVector::DotProduct(N, FVector::UpVector);

                    Near++;
                    if (UpDot < 0.f) Over++;
                    if (FMath::Abs(UpDot) <= S.SteepDotThreshold) Steep++;
                }
            }
        }
    }

    M.CaveSamples = CaveTotal;
    M.CaveVoidRatio = (CaveTotal > 0) ? (float)CaveAir / (float)CaveTotal : 0.f;

    M.SurfaceNearSamples = Near;
    M.OverhangRatio = (Near > 0) ? (float)Over / (float)Near : 0.f;
    M.SteepRatio = (Near > 0) ? (float)Steep / (float)Near : 0.f;

    return M;
}