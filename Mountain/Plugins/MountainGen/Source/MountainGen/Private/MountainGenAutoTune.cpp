#include "MountainGenAutoTune.h"
#include "VoxelDensityGenerator.h"
#include "HAL/PlatformTime.h"

static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }
static FORCEINLINE bool InRange(float v, float a, float b) { return (v >= a && v <= b); }
static FORCEINLINE float RangeMid(float a, float b) { return 0.5f * (a + b); }

static FORCEINLINE float ScoreToRange(float v, float mn, float mx)
{
    if (v < mn) return (mn - v);
    if (v > mx) return (v - mx);
    return 0.f;
}

FMGDifficultyPreset MGMakeDifficultyPreset(EMountainGenDifficulty D)
{
    FMGDifficultyPreset P;
    auto& B = P.Bounds;

    switch (D)
    {
    case EMountainGenDifficulty::Easy:
        P.Targets.OverhangMin = 0.00f; P.Targets.OverhangMax = 0.05f;
        P.Targets.SteepMin = 0.05f; P.Targets.SteepMax = 0.20f;

        P.BaseField3DStrengthCm = 5000.f;
        P.BaseField3DScaleCm = 22000.f;
        P.DetailScaleCm = 9000.f;

        P.VolumeStrength = 0.20f;
        P.OverhangScaleCm = 14000.f;
        P.OverhangFadeCm = 28000.f;
        P.OverhangBias = 0.70f;
        P.OverhangDepthCm = 1500.f;

        P.GravityStrength = 1.30f;
        P.GravityScale = 2.5f;

        B.BaseField3DStrengthCm = { 2000.f, 7000.f };
        B.BaseField3DScaleCm = { 18000.f, 30000.f };
        B.DetailScaleCm = { 7000.f, 14000.f };

        B.VolumeStrength = { 0.0f, 0.4f };
        B.OverhangScaleCm = { 10000.f, 20000.f };
        B.OverhangBias = { 0.62f, 0.80f };
        B.OverhangDepthCm = { 500.f, 2500.f };
        B.OverhangFadeCm = { 20000.f, 40000.f };

        B.GravityStrength = { 1.10f, 1.60f };
        B.GravityScale = { 2.0f, 3.2f };
        break;

    case EMountainGenDifficulty::Normal:
        P.Targets.OverhangMin = 0.02f; P.Targets.OverhangMax = 0.10f;
        P.Targets.SteepMin = 0.15f; P.Targets.SteepMax = 0.35f;

        P.BaseField3DStrengthCm = 8500.f;
        P.BaseField3DScaleCm = 18000.f;
        P.DetailScaleCm = 7000.f;

        P.VolumeStrength = 0.55f;
        P.OverhangScaleCm = 10000.f;
        P.OverhangFadeCm = 20000.f;
        P.OverhangBias = 0.60f;
        P.OverhangDepthCm = 2500.f;

        P.GravityStrength = 1.10f;
        P.GravityScale = 2.2f;

        B.BaseField3DStrengthCm = { 5000.f, 12000.f };
        B.BaseField3DScaleCm = { 14000.f, 24000.f };
        B.DetailScaleCm = { 5000.f, 10000.f };

        B.VolumeStrength = { 0.25f, 0.9f };
        B.OverhangScaleCm = { 7000.f, 14000.f };
        B.OverhangBias = { 0.52f, 0.68f };
        B.OverhangDepthCm = { 1200.f, 4500.f };
        B.OverhangFadeCm = { 12000.f, 26000.f };

        B.GravityStrength = { 0.95f, 1.30f };
        B.GravityScale = { 1.6f, 2.8f };
        break;

    case EMountainGenDifficulty::Hard:
        P.Targets.OverhangMin = 0.06f; P.Targets.OverhangMax = 0.18f;
        P.Targets.SteepMin = 0.25f; P.Targets.SteepMax = 0.55f;

        P.BaseField3DStrengthCm = 12000.f;
        P.BaseField3DScaleCm = 16000.f;
        P.DetailScaleCm = 6000.f;

        P.VolumeStrength = 1.00f;
        P.OverhangScaleCm = 8000.f;
        P.OverhangFadeCm = 15000.f;
        P.OverhangBias = 0.55f;
        P.OverhangDepthCm = 3500.f;

        P.GravityStrength = 1.00f;
        P.GravityScale = 2.0f;

        B.BaseField3DStrengthCm = { 9000.f, 16000.f };
        B.BaseField3DScaleCm = { 11000.f, 20000.f };
        B.DetailScaleCm = { 3500.f, 8000.f };

        B.VolumeStrength = { 0.75f, 1.4f };
        B.OverhangScaleCm = { 5000.f, 11000.f };
        B.OverhangBias = { 0.45f, 0.62f };
        B.OverhangDepthCm = { 2000.f, 6500.f };
        B.OverhangFadeCm = { 9000.f, 20000.f };

        B.GravityStrength = { 0.85f, 1.10f };
        B.GravityScale = { 1.3f, 2.4f };
        break;

    default: // Extreme
        P.Targets.OverhangMin = 0.12f; P.Targets.OverhangMax = 0.30f;
        P.Targets.SteepMin = 0.40f; P.Targets.SteepMax = 0.80f;

        P.BaseField3DStrengthCm = 17000.f;
        P.BaseField3DScaleCm = 12000.f;
        P.DetailScaleCm = 4200.f;

        P.VolumeStrength = 1.60f;
        P.OverhangScaleCm = 6000.f;
        P.OverhangFadeCm = 9000.f;
        P.OverhangBias = 0.48f;
        P.OverhangDepthCm = 5200.f;

        P.GravityStrength = 0.85f;
        P.GravityScale = 1.7f;

        B.BaseField3DStrengthCm = { 13000.f, 22000.f };
        B.BaseField3DScaleCm = { 8000.f, 15000.f };
        B.DetailScaleCm = { 2500.f, 5500.f };

        B.VolumeStrength = { 1.2f, 2.0f };
        B.OverhangScaleCm = { 3500.f, 8000.f };
        B.OverhangBias = { 0.35f, 0.55f };
        B.OverhangDepthCm = { 3500.f, 8000.f };
        B.OverhangFadeCm = { 6000.f, 14000.f };

        B.GravityStrength = { 0.55f, 0.95f };
        B.GravityScale = { 1.0f, 2.0f };
        break;
    }

    return P;
}

void MGClampToDifficultyBounds(FMountainGenSettings& S)
{
    const FMGDifficultyPreset P = MGMakeDifficultyPreset(S.Difficulty);
    const auto& B = P.Bounds;

    S.BaseField3DStrengthCm = B.BaseField3DStrengthCm.Clamp(S.BaseField3DStrengthCm);
    S.BaseField3DScaleCm = B.BaseField3DScaleCm.Clamp(S.BaseField3DScaleCm);
    S.DetailScaleCm = B.DetailScaleCm.Clamp(S.DetailScaleCm);

    S.VolumeStrength = B.VolumeStrength.Clamp(S.VolumeStrength);
    S.OverhangScaleCm = B.OverhangScaleCm.Clamp(S.OverhangScaleCm);
    S.OverhangBias = B.OverhangBias.Clamp(S.OverhangBias);
    S.OverhangDepthCm = B.OverhangDepthCm.Clamp(S.OverhangDepthCm);
    S.OverhangFadeCm = B.OverhangFadeCm.Clamp(S.OverhangFadeCm);

    S.GravityStrength = B.GravityStrength.Clamp(S.GravityStrength);
    S.GravityScale = B.GravityScale.Clamp(S.GravityScale);
}

void MGApplyDifficultyPreset(FMountainGenSettings& S)
{
    const FMGDifficultyPreset P = MGMakeDifficultyPreset(S.Difficulty);

    S.Targets = P.Targets;

    // 의도 파라미터 초기점
    S.BaseField3DStrengthCm = P.BaseField3DStrengthCm;
    S.BaseField3DScaleCm = P.BaseField3DScaleCm;
    S.DetailScaleCm = P.DetailScaleCm;

    S.VolumeStrength = P.VolumeStrength;
    S.OverhangScaleCm = P.OverhangScaleCm;
    S.OverhangBias = P.OverhangBias;
    S.OverhangDepthCm = P.OverhangDepthCm;
    S.OverhangFadeCm = P.OverhangFadeCm;

    S.GravityStrength = P.GravityStrength;
    S.GravityScale = P.GravityScale;

    MGClampToDifficultyBounds(S);
}

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    FMGMetrics M;

    const int32 N = FMath::Max(16, S.MetricsSamplesPerTry);
    const float Step = (S.MetricsStepCm > 0.f) ? S.MetricsStepCm : FMath::Max(400.f, S.VoxelSizeCm * 2.f);

    const float Iso = S.IsoLevel;
    const float NearBand = FMath::Max(200.f, Step * 0.6f);

    const float Dx = Step;
    const float Inv2Dx = 1.f / (2.f * Dx);

    FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

    int32 NearCnt = 0;
    int32 SteepCnt = 0;
    int32 OverCnt = 0;

    FRandomStream Rng((int32)(S.Seed ^ 0xA1B2C3D4));

    for (int32 i = 0; i < N; ++i)
    {
        const float x = Rng.FRandRange(WorldMin.X, WorldMax.X);
        const float y = Rng.FRandRange(WorldMin.Y, WorldMax.Y);
        const float z = Rng.FRandRange(WorldMin.Z, WorldMax.Z);

        const FVector P(x, y, z);
        const float d = Gen.SampleDensity(P);

        if (FMath::Abs(d - Iso) > NearBand) continue;
        NearCnt++;

        const float dxp = Gen.SampleDensity(P + FVector(Dx, 0, 0));
        const float dxm = Gen.SampleDensity(P - FVector(Dx, 0, 0));
        const float dyp = Gen.SampleDensity(P + FVector(0, Dx, 0));
        const float dym = Gen.SampleDensity(P - FVector(0, Dx, 0));
        const float dzp = Gen.SampleDensity(P + FVector(0, 0, Dx));
        const float dzm = Gen.SampleDensity(P - FVector(0, 0, Dx));

        const FVector G(
            (dxp - dxm) * Inv2Dx,
            (dyp - dym) * Inv2Dx,
            (dzp - dzm) * Inv2Dx
        );

        const float g2 = G.SizeSquared();
        const float g = FMath::Sqrt(FMath::Max(0.f, g2));

        if (g > 0.025f) SteepCnt++;

        // density-field normal ~ -grad
        const FVector Nrm = (g > 1e-6f) ? (-G / g) : FVector::UpVector;
        if (Nrm.Z < -0.15f) OverCnt++;
    }

    M.SurfaceNearSamples = NearCnt;
    if (NearCnt > 0)
    {
        M.SteepRatio = Clamp01((float)SteepCnt / (float)NearCnt);
        M.OverhangRatio = Clamp01((float)OverCnt / (float)NearCnt);
    }

    return M;
}

void MGAutoTuneIntentParams(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    MGClampToDifficultyBounds(InOutS);

    const int32 Iters = FMath::Clamp(InOutS.FeedbackIters, 0, 30);
    if (Iters <= 0) return;

    const float TargetOver = RangeMid(InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
    const float TargetSteep = RangeMid(InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);

    for (int32 it = 0; it < Iters; ++it)
    {
        const FMGMetrics M = MGComputeMetricsQuick(InOutS, TerrainOriginWorld, WorldMin, WorldMax);

        const float eOver = (TargetOver - M.OverhangRatio);
        const float eSteep = (TargetSteep - M.SteepRatio);

        const float kO = 0.35f;
        const float kS = 0.25f;

        InOutS.VolumeStrength += kO * eOver;
        InOutS.OverhangDepthCm += (kO * eOver) * 1800.f;
        InOutS.OverhangBias -= (kO * eOver) * 0.10f;
        InOutS.OverhangFadeCm += (kO * eOver) * 3500.f;

        InOutS.BaseField3DStrengthCm += (kS * eSteep) * 4500.f;
        InOutS.DetailScaleCm -= (kS * eSteep) * 1800.f;

        MGClampToDifficultyBounds(InOutS);

        const bool bOverOK = InRange(M.OverhangRatio, InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
        const bool bSteepOK = InRange(M.SteepRatio, InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);
        if (bOverOK && bSteepOK) break;
    }
}

int32 MGSearchSeedForTargets(
    const FMountainGenSettings& BaseS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax,
    int32 InputSeed,
    int32 Tries,
    bool bRetryUntilSatisfied,
    int32 MaxAttempts,
    bool bDebug,
    int32 DebugEveryN,
    TFunction<void(const FString&, float, FColor)> DebugPrint)
{
    const int32 WantedTries = FMath::Max(1, Tries);
    const int32 MaxTry = bRetryUntilSatisfied ? FMath::Max(1, MaxAttempts) : WantedTries;

    FRandomStream Rng;
    if (InputSeed > 0) Rng.Initialize(InputSeed ^ 0x1F3A9B2D);
    else Rng.Initialize((int32)FPlatformTime::Cycles64());

    int32 BestSeed = (InputSeed > 0) ? InputSeed : Rng.RandRange(1, INT32_MAX);
    float BestScore = FLT_MAX;

    for (int32 Attempt = 1; Attempt <= MaxTry; ++Attempt)
    {
        const int32 Seed =
            (Attempt == 1 && InputSeed > 0) ? InputSeed : Rng.RandRange(1, INT32_MAX);

        FMountainGenSettings Cand = BaseS;
        Cand.Seed = Seed;

        MGDeriveReproducibleDomainFromSeed(Cand, Seed);

        const FMGMetrics M = MGComputeMetricsQuick(Cand, TerrainOriginWorld, WorldMin, WorldMax);

        const bool bOverOK = InRange(M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax);
        const bool bSteepOK = InRange(M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax);

        const float Score =
            ScoreToRange(M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax) +
            ScoreToRange(M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestSeed = Seed;
        }

        if (bDebug && DebugPrint)
        {
            const int32 EveryN = FMath::Max(1, DebugEveryN);
            const bool bPrint = (Attempt == 1) || (Attempt == MaxTry) || ((Attempt % EveryN) == 0);

            if (bPrint)
            {
                const FString Line = FString::Printf(
                    TEXT("seed=%d | Over=%.3f [%.2f~%.2f] %s | Steep=%.3f [%.2f~%.2f] %s | Near=%d | Score=%.3f"),
                    Seed,
                    M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax, bOverOK ? TEXT("OK") : TEXT("FAIL"),
                    M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax, bSteepOK ? TEXT("OK") : TEXT("FAIL"),
                    M.SurfaceNearSamples,
                    Score
                );

                FColor C = FColor::Red;
                if (bOverOK && bSteepOK) C = FColor::Green;
                else if (!bOverOK && bSteepOK) C = FColor::Blue;
                else if (bOverOK && !bSteepOK) C = FColor::Yellow;

                DebugPrint(FString::Printf(TEXT("[SeedSearch] %d/%d %s"), Attempt, MaxTry, *Line), 4.5f, C);
            }
        }

        if (bOverOK && bSteepOK) return Seed;
    }

    return BestSeed;
}

void MGDeriveReproducibleDomainFromSeed(FMountainGenSettings& InOutS, int32 Seed)
{
    InOutS.Seed = FMath::Max(1, Seed);
}