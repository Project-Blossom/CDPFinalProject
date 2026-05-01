#include "MountainGenAutoTune.h"
#include "GDPCGScoring.h"
#include "VoxelDensityGenerator.h"
#include "HAL/PlatformTime.h"
#include "Async/ParallelFor.h"

// ============================================================
// Utils
// ============================================================

static FORCEINLINE float Clamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }
static FORCEINLINE bool InRange(float v, float a, float b) { return (v >= a && v <= b); }
static FORCEINLINE float RangeMid(float a, float b) { return 0.5f * (a + b); }

static FORCEINLINE float ScoreToRange(float v, float mn, float mx)
{
    if (v < mn) return (mn - v);
    if (v > mx) return (v - mx);
    return 0.f;
}

// ============================================================
// Metrics helpers
// ============================================================

static FORCEINLINE float GetSteepDotThreshold(const FMountainGenSettings& S)
{
    static constexpr float kDefaultSteepDot = 0.17f;

    if (S.SteepDotOverride >= 0.f)
        return FMath::Clamp(S.SteepDotOverride, 0.f, 1.f);

    return kDefaultSteepDot;
}

static FVector EstimateNormalCentralDiff(
    const FVoxelDensityGenerator& Gen,
    const FVector& P,
    float e)
{
    const float dx = Gen.SampleDensity(P + FVector(e, 0, 0)) - Gen.SampleDensity(P - FVector(e, 0, 0));
    const float dy = Gen.SampleDensity(P + FVector(0, e, 0)) - Gen.SampleDensity(P - FVector(0, e, 0));
    const float dz = Gen.SampleDensity(P + FVector(0, 0, e)) - Gen.SampleDensity(P - FVector(0, 0, e));
    return FVector(dx, dy, dz).GetSafeNormal();
}

static bool FindIsoCrossingAlongX_Cached(
    const FVoxelDensityGenerator& Gen,
    float Iso,
    FVector A, float dA,
    FVector B, float dB,
    int32 Steps,
    FVector& OutHit)
{
    if (dA * dB > 0.f) return false;

    FVector a = A;
    FVector b = B;
    float d0 = dA;
    float d1 = dB;

    Steps = FMath::Clamp(Steps, 6, 24);
    for (int32 i = 0; i < Steps; ++i)
    {
        const FVector m = (a + b) * 0.5f;
        const float dm = Gen.SampleDensity(m) - Iso;

        if (d0 * dm <= 0.f) { b = m; d1 = dm; }
        else { a = m; d0 = dm; }
    }

    OutHit = (a + b) * 0.5f;
    return true;
}

// ============================================================
// Metrics context
// ============================================================

struct FMGMetricsContext
{
    int32 Grid = 0;
    TArray<float> Ys;
    TArray<float> Zs;

    float Iso = 0.f;
    float FrontX = 0.f;
    float Voxel = 0.f;
    float SurfaceBand = 0.f;
    float SteepDot = 0.f;
    float e = 0.f;

    FVector WorldMin;
    FVector WorldMax;
    FVector TerrainOriginWorld;

    static constexpr float BaseCliff_NxAbs_Threshold = 0.97f;
    static constexpr float BaseCliff_UpAbs_Threshold = 0.12f;
    static constexpr float BaseCliff_NyAbs_Threshold = 0.25f;

    void Build(const FMountainGenSettings& S,
        const FVector& InTerrainOriginWorld,
        const FVector& InWorldMin,
        const FVector& InWorldMax)
    {
        TerrainOriginWorld = InTerrainOriginWorld;
        WorldMin = InWorldMin;
        WorldMax = InWorldMax;

        Iso = S.IsoLevel;

        const int32 TotalRays = FMath::Clamp(S.MetricsSamplesPerTry, 16, 4096);
        Grid = FMath::Max(4, (int32)FMath::Sqrt((float)TotalRays));

        Ys.SetNumUninitialized(Grid);
        Zs.SetNumUninitialized(Grid);

        const float y0 = WorldMin.Y;
        const float y1 = WorldMax.Y;
        const float z0 = WorldMin.Z;
        const float z1 = WorldMax.Z;

        for (int32 i = 0; i < Grid; ++i)
        {
            const float ty = (i + 0.5f) / (float)Grid;
            const float tz = (i + 0.5f) / (float)Grid;
            Ys[i] = FMath::Lerp(y0, y1, ty);
            Zs[i] = FMath::Lerp(z0, z1, tz);
        }

        FrontX = FMath::Max(200.f, S.CliffThicknessCm);
        Voxel = FMath::Max(1.f, S.VoxelSizeCm);
        SurfaceBand = FMath::Max(Voxel * 3.f, FMath::Min(S.OverhangFadeCm, 6000.f));
        SteepDot = GetSteepDotThreshold(S);
        e = FMath::Max(5.f, Voxel * 0.25f);
    }

    FMGMetrics Compute(const FMountainGenSettings& S) const
    {
        FMGMetrics M{};

        const FVoxelDensityGenerator Gen(S, TerrainOriginWorld);
        const FVector Up(0, 0, 1);

        // 병렬 누적용 (원자 연산 피하려고 TLS 합산)
        struct FAcc
        {
            int32 Valid = 0;
            int32 Over = 0;
            int32 Steep = 0;
            int32 Rough = 0;
            int32 ShadowRisk = 0;
        };
        TArray<FAcc> Accs;
        const int32 NumWorkers = FMath::Max(1, FTaskGraphInterface::Get().GetNumWorkerThreads());
        Accs.SetNumZeroed(NumWorkers + 1);

        const int32 Total = Grid * Grid;

        ParallelFor(Total, [&](int32 idx)
            {
                const int32 Worker = FMath::Clamp(FPlatformTLS::GetCurrentThreadId() % (NumWorkers + 1), 0, NumWorkers);
                FAcc& A = Accs[Worker];

                const int32 iz = idx / Grid;
                const int32 iy = idx - iz * Grid;

                const float z = Zs[iz];
                const float y = Ys[iy];

                const FVector P0(WorldMin.X, y, z);
                const FVector P1(WorldMax.X, y, z);

                const float dP0 = Gen.SampleDensity(P0) - Iso;
                const float dP1 = Gen.SampleDensity(P1) - Iso;

                FVector Outside = P0;
                FVector Inside = P1;
                float dOut = dP0;
                float dIn = dP1;

                if (dP0 < 0.f && dP1 >= 0.f)
                {
                }
                else if (dP1 < 0.f && dP0 >= 0.f)
                {
                    Outside = P1; Inside = P0;
                    dOut = dP1;  dIn = dP0;
                }
                else
                {
                    return;
                }

                FVector Hit;
                if (!FindIsoCrossingAlongX_Cached(Gen, Iso, Outside, dOut, Inside, dIn, 14, Hit))
                    return;

                const float LocalX = Hit.X - TerrainOriginWorld.X;
                const float insideDepth = (FrontX - LocalX);

                if (insideDepth < 0.f || insideDepth > SurfaceBand)
                    return;

                const FVector N = EstimateNormalCentralDiff(Gen, Hit, e);
                if (N.IsNearlyZero())
                    return;

                const float UpDot = FVector::DotProduct(N, Up);

                // Surface quality metric:
                // Compare the hit normal with nearby normals in Y/Z directions.
                // This catches high-frequency tearing that Overhang/Steep alone cannot distinguish.
                const float qStep = FMath::Max(e * 1.5f, Voxel * 0.5f);
                const FVector Ny = EstimateNormalCentralDiff(Gen, Hit + FVector(0.f, qStep, 0.f), e);
                const FVector Nz = EstimateNormalCentralDiff(Gen, Hit + FVector(0.f, 0.f, qStep), e);

                float NormalDelta = 0.f;
                if (!Ny.IsNearlyZero())
                {
                    NormalDelta = FMath::Max(NormalDelta, 1.f - FMath::Clamp(FVector::DotProduct(N, Ny), -1.f, 1.f));
                }
                if (!Nz.IsNearlyZero())
                {
                    NormalDelta = FMath::Max(NormalDelta, 1.f - FMath::Clamp(FVector::DotProduct(N, Nz), -1.f, 1.f));
                }

                const float RoughDeltaThreshold = 0.18f;
                const float ShadowDeltaThreshold = 0.30f;

                const float NxAbs = FMath::Abs(N.X);
                const float NyAbs = FMath::Abs(N.Y);
                const bool bIsBaseCliff =
                    (NxAbs >= BaseCliff_NxAbs_Threshold) &&
                    (FMath::Abs(UpDot) <= BaseCliff_UpAbs_Threshold) &&
                    (NyAbs <= BaseCliff_NyAbs_Threshold);

                if (bIsBaseCliff)
                    return;

                A.Valid++;

                if (NormalDelta >= RoughDeltaThreshold)
                {
                    A.Rough++;
                }

                if (NormalDelta >= ShadowDeltaThreshold)
                {
                    A.ShadowRisk++;
                }

                static constexpr float OverhangUpDotThreshold = -0.15f;
                if (UpDot <= OverhangUpDotThreshold) A.Over++;
                if (FMath::Abs(UpDot) <= SteepDot)   A.Steep++;
            });

        int32 Valid = 0, Over = 0, Steep = 0, Rough = 0, ShadowRisk = 0;
        for (const FAcc& A : Accs)
        {
            Valid += A.Valid;
            Over += A.Over;
            Steep += A.Steep;
            Rough += A.Rough;
            ShadowRisk += A.ShadowRisk;
        }

        M.SurfaceNearSamples = Valid;
        if (Valid > 0)
        {
            M.OverhangRatio = (float)Over / (float)Valid;
            M.SteepRatio = (float)Steep / (float)Valid;
            M.RoughnessRatio = (float)Rough / (float)Valid;
            M.ShadowRiskRatio = (float)ShadowRisk / (float)Valid;
        }
        else
        {
            M.OverhangRatio = 0.f;
            M.SteepRatio = 0.f;
            M.RoughnessRatio = 0.f;
            M.ShadowRiskRatio = 0.f;
        }

        return M;
    }
};

// ============================================================
// Metrics
// ============================================================

FMGMetrics MGComputeMetrics_RaycastYZ(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    FMGMetricsContext Ctx;
    Ctx.Build(S, TerrainOriginWorld, WorldMin, WorldMax);
    return Ctx.Compute(S);
}

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    return MGComputeMetrics_RaycastYZ(S, TerrainOriginWorld, WorldMin, WorldMax);
}

// ============================================================================
// Difficulty Preset
// ============================================================================

FMGDifficultyPreset MGMakeDifficultyPreset(EMountainGenDifficulty D)
{
    FMGDifficultyPreset P;
    auto& B = P.Bounds;

    switch (D)
    {
    case EMountainGenDifficulty::Easy:
        P.Targets.OverhangMin = 0.12f; P.Targets.OverhangMax = 0.38f;
        P.Targets.SteepMin = 0.15f; P.Targets.SteepMax = 0.35f;
        P.Targets.RoughnessMax = 0.12f;
        P.Targets.ShadowRiskMax = 0.035f;

        P.BaseField3DStrengthCm = 6500.f;
        P.BaseField3DScaleCm = 22000.f;
        P.DetailScaleCm = 8000.f;
        P.DetailStrengthCm = 520.f;
        P.SurfaceRoughnessStrengthCm = 160.f;
        P.SurfaceRoughnessMaskStrength = 0.88f;
        P.SurfaceQualityScoreWeight = 1.60f;

        P.VolumeStrength = 0.35f;
        P.OverhangScaleCm = 14000.f;
        P.OverhangFadeCm = 24000.f;
        P.OverhangBias = 0.72f;
        P.OverhangDepthCm = 2200.f;


        B.BaseField3DStrengthCm = { 3000.f, 9000.f };
        B.BaseField3DScaleCm = { 18000.f, 30000.f };
        B.DetailScaleCm = { 6500.f, 14000.f };
        B.DetailStrengthCm = { 300.f, 700.f };
        B.SurfaceRoughnessStrengthCm = { 0.f, 220.f };
        B.SurfaceRoughnessMaskStrength = { 0.75f, 0.95f };
        B.SurfaceQualityScoreWeight = { 1.20f, 2.00f };

        B.VolumeStrength = { 0.10f, 0.55f };
        B.OverhangScaleCm = { 10000.f, 20000.f };
        B.OverhangBias = { 0.66f, 0.82f };
        B.OverhangDepthCm = { 800.f, 3200.f };
        B.OverhangFadeCm = { 18000.f, 32000.f };

        break;

    case EMountainGenDifficulty::Normal:
        P.Targets.OverhangMin = 0.18f; P.Targets.OverhangMax = 0.45f;
        P.Targets.SteepMin = 0.18f; P.Targets.SteepMax = 0.40f;
        P.Targets.RoughnessMax = 0.16f;
        P.Targets.ShadowRiskMax = 0.055f;

        P.BaseField3DStrengthCm = 8500.f;
        P.BaseField3DScaleCm = 18000.f;
        P.DetailScaleCm = 7000.f;
        P.DetailStrengthCm = 760.f;
        P.SurfaceRoughnessStrengthCm = 220.f;
        P.SurfaceRoughnessMaskStrength = 0.78f;
        P.SurfaceQualityScoreWeight = 1.40f;

        P.VolumeStrength = 0.55f;
        P.OverhangScaleCm = 10000.f;
        P.OverhangFadeCm = 20000.f;
        P.OverhangBias = 0.60f;
        P.OverhangDepthCm = 2500.f;


        B.BaseField3DStrengthCm = { 5000.f, 12000.f };
        B.BaseField3DScaleCm = { 14000.f, 24000.f };
        B.DetailScaleCm = { 5000.f, 10000.f };
        B.DetailStrengthCm = { 500.f, 1000.f };
        B.SurfaceRoughnessStrengthCm = { 100.f, 320.f };
        B.SurfaceRoughnessMaskStrength = { 0.65f, 0.90f };
        B.SurfaceQualityScoreWeight = { 1.00f, 1.80f };

        B.VolumeStrength = { 0.25f, 0.9f };
        B.OverhangScaleCm = { 7000.f, 14000.f };
        B.OverhangBias = { 0.52f, 0.68f };
        B.OverhangDepthCm = { 1200.f, 4500.f };
        B.OverhangFadeCm = { 12000.f, 26000.f };

        break;

    case EMountainGenDifficulty::Hard:
    default:
        P.Targets.OverhangMin = 0.28f; P.Targets.OverhangMax = 0.60f;
        P.Targets.SteepMin = 0.25f; P.Targets.SteepMax = 0.55f;
        P.Targets.RoughnessMax = 0.22f;
        P.Targets.ShadowRiskMax = 0.080f;

        P.BaseField3DStrengthCm = 12000.f;
        P.BaseField3DScaleCm = 16000.f;
        P.DetailScaleCm = 6000.f;
        P.DetailStrengthCm = 1100.f;
        P.SurfaceRoughnessStrengthCm = 320.f;
        P.SurfaceRoughnessMaskStrength = 0.70f;
        P.SurfaceQualityScoreWeight = 1.10f;

        P.VolumeStrength = 1.00f;
        P.OverhangScaleCm = 8000.f;
        P.OverhangFadeCm = 15000.f;
        P.OverhangBias = 0.55f;
        P.OverhangDepthCm = 3500.f;


        B.BaseField3DStrengthCm = { 9000.f, 16000.f };
        B.BaseField3DScaleCm = { 11000.f, 20000.f };
        B.DetailScaleCm = { 3500.f, 8000.f };
        B.DetailStrengthCm = { 800.f, 1500.f };
        B.SurfaceRoughnessStrengthCm = { 160.f, 450.f };
        B.SurfaceRoughnessMaskStrength = { 0.55f, 0.85f };
        B.SurfaceQualityScoreWeight = { 0.80f, 1.50f };

        B.VolumeStrength = { 0.75f, 1.4f };
        B.OverhangScaleCm = { 5000.f, 11000.f };
        B.OverhangBias = { 0.45f, 0.62f };
        B.OverhangDepthCm = { 2000.f, 6500.f };
        B.OverhangFadeCm = { 9000.f, 20000.f };

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
    S.DetailStrengthCm = B.DetailStrengthCm.Clamp(S.DetailStrengthCm);
    S.SurfaceRoughnessStrengthCm = B.SurfaceRoughnessStrengthCm.Clamp(S.SurfaceRoughnessStrengthCm);
    S.SurfaceRoughnessMaskStrength = B.SurfaceRoughnessMaskStrength.Clamp(S.SurfaceRoughnessMaskStrength);
    S.SurfaceQualityScoreWeight = B.SurfaceQualityScoreWeight.Clamp(S.SurfaceQualityScoreWeight);

    S.VolumeStrength = B.VolumeStrength.Clamp(S.VolumeStrength);
    S.OverhangScaleCm = B.OverhangScaleCm.Clamp(S.OverhangScaleCm);
    S.OverhangBias = B.OverhangBias.Clamp(S.OverhangBias);
    S.OverhangDepthCm = B.OverhangDepthCm.Clamp(S.OverhangDepthCm);
    S.OverhangFadeCm = B.OverhangFadeCm.Clamp(S.OverhangFadeCm);

}

void MGApplyDifficultyPreset(FMountainGenSettings& S)
{
    const FMGDifficultyPreset P = MGMakeDifficultyPreset(S.Difficulty);

    S.Targets = P.Targets;

    S.BaseField3DStrengthCm = P.BaseField3DStrengthCm;
    S.BaseField3DScaleCm = P.BaseField3DScaleCm;
    S.DetailScaleCm = P.DetailScaleCm;
    S.DetailStrengthCm = P.DetailStrengthCm;
    S.SurfaceRoughnessStrengthCm = P.SurfaceRoughnessStrengthCm;
    S.SurfaceRoughnessMaskStrength = P.SurfaceRoughnessMaskStrength;
    S.SurfaceQualityScoreWeight = P.SurfaceQualityScoreWeight;

    S.VolumeStrength = P.VolumeStrength;
    S.OverhangScaleCm = P.OverhangScaleCm;
    S.OverhangBias = P.OverhangBias;
    S.OverhangDepthCm = P.OverhangDepthCm;
    S.OverhangFadeCm = P.OverhangFadeCm;


    MGClampToDifficultyBounds(S);
}

// ============================================================
// AutoTune
// ============================================================

void MGAutoTuneIntentParams(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    MGClampToDifficultyBounds(InOutS);

    const int32 Iters = FMath::Clamp(InOutS.FeedbackIters, 0, 30);
    if (Iters <= 0) return;

    FMGMetricsContext Ctx;
    Ctx.Build(InOutS, TerrainOriginWorld, WorldMin, WorldMax);

    const float TargetOver = RangeMid(InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
    const float TargetSteep = RangeMid(InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);

    for (int32 it = 0; it < Iters; ++it)
    {
        const FMGMetrics M = Ctx.Compute(InOutS);

        const float eOver = (TargetOver - M.OverhangRatio);
        const float eSteep = (TargetSteep - M.SteepRatio);
        const float eRough = FMath::Max(0.f, M.RoughnessRatio - InOutS.Targets.RoughnessMax);
        const float eShadow = FMath::Max(0.f, M.ShadowRiskRatio - InOutS.Targets.ShadowRiskMax);

        const float kO = 0.35f;
        const float kS = 0.25f;

        InOutS.VolumeStrength += kO * eOver;
        InOutS.OverhangDepthCm += (kO * eOver) * 1800.f;
        InOutS.OverhangBias -= (kO * eOver) * 0.10f;
        InOutS.OverhangFadeCm += (kO * eOver) * 3500.f;

        InOutS.BaseField3DStrengthCm += (kS * eSteep) * 4500.f;
        InOutS.DetailScaleCm -= (kS * eSteep) * 1800.f;

        if (eRough > 0.f || eShadow > 0.f)
        {
            const float QualityError = eRough + eShadow * 1.5f;
            InOutS.DetailStrengthCm -= QualityError * 1200.f;
            InOutS.SurfaceRoughnessStrengthCm -= QualityError * 900.f;
            InOutS.SurfaceRoughnessMaskStrength += QualityError * 0.40f;
            InOutS.DetailScaleCm += QualityError * 2200.f;
        }

        MGClampToDifficultyBounds(InOutS);

        const bool bOverOK = InRange(M.OverhangRatio, InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
        const bool bSteepOK = InRange(M.SteepRatio, InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);
        const bool bRoughOK = (M.RoughnessRatio <= InOutS.Targets.RoughnessMax);
        const bool bShadowOK = (M.ShadowRiskRatio <= InOutS.Targets.ShadowRiskMax);
        if (bOverOK && bSteepOK && bRoughOK && bShadowOK) break;
    }
}

// ============================================================
// GoalDrivenPCG adapter for MountainGen cliff metrics
// 절벽 전용 Metrics를 목표 기반 제어형 PCG 공통 구조로 변환한다.
// ============================================================

static FGDPCGTargetProfile MGMakeGoalProfileFromSettings(const FMountainGenSettings& S)
{
    FGDPCGTargetProfile Profile;

    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:
        Profile.ProfileName = TEXT("MountainGen.Easy");
        break;
    case EMountainGenDifficulty::Normal:
        Profile.ProfileName = TEXT("MountainGen.Normal");
        break;
    case EMountainGenDifficulty::Hard:
    default:
        Profile.ProfileName = TEXT("MountainGen.Hard");
        break;
    }

    Profile.Targets.Add(FGDPCGMetricTarget{
        TEXT("OverhangRatio"),
        EGDPCGMetricGoalType::Range,
        S.Targets.OverhangMin,
        S.Targets.OverhangMax,
        1.0f
        });

    Profile.Targets.Add(FGDPCGMetricTarget{
        TEXT("SteepRatio"),
        EGDPCGMetricGoalType::Range,
        S.Targets.SteepMin,
        S.Targets.SteepMax,
        1.0f
        });

    Profile.Targets.Add(FGDPCGMetricTarget{
        TEXT("RoughnessRatio"),
        EGDPCGMetricGoalType::LowerIsBetter,
        0.0f,
        S.Targets.RoughnessMax,
        FMath::Max(0.0f, S.SurfaceQualityScoreWeight)
        });

    Profile.Targets.Add(FGDPCGMetricTarget{
        TEXT("ShadowRiskRatio"),
        EGDPCGMetricGoalType::LowerIsBetter,
        0.0f,
        S.Targets.ShadowRiskMax,
        FMath::Max(0.0f, S.SurfaceQualityScoreWeight * 1.5f)
        });

    return Profile;
}

static TArray<FGDPCGMetricValue> MGMakeObservedMetricsFromMetrics(const FMGMetrics& M)
{
    TArray<FGDPCGMetricValue> Out;
    Out.Reserve(4);

    auto AddMetric = [&Out](const TCHAR* Name, float Value)
        {
            FGDPCGMetricValue Metric;
            Metric.MetricName = FName(Name);
            Metric.Value = Value;
            Out.Add(Metric);
        };

    AddMetric(TEXT("OverhangRatio"), M.OverhangRatio);
    AddMetric(TEXT("SteepRatio"), M.SteepRatio);
    AddMetric(TEXT("RoughnessRatio"), M.RoughnessRatio);
    AddMetric(TEXT("ShadowRiskRatio"), M.ShadowRiskRatio);

    return Out;
}

// ============================================================
// Hierarchical Goal Search
// ============================================================

struct FMGSeedCandidate
{
    int32 Seed = 1;
    FMountainGenSettings Settings;
    FMGMetrics ProxyMetrics;
    FMGMetrics FullMetrics;
    float ProxyScore = FLT_MAX;
    float FullScore = FLT_MAX;
    bool bProxyPassed = false;
    bool bFullPassed = false;
};

static float MGComputeGoalScore(const FMountainGenSettings& S, const FMGMetrics& M, bool& bOutPassed)
{
    const FGDPCGTargetProfile GoalProfile = MGMakeGoalProfileFromSettings(S);
    const TArray<FGDPCGMetricValue> ObservedMetrics = MGMakeObservedMetricsFromMetrics(M);
    const FGDPCGCandidateResult GoalResult = GDPCGScoring::EvaluateCandidate(S.Seed, GoalProfile, ObservedMetrics);

    float Score = GoalResult.FinalScore;

    // Hard constraints: 렌더링/표면 안정성 계열은 목표 상한을 넘으면 훨씬 강하게 벌점 처리한다.
    // 이렇게 해야 Overhang/Steep만 맞고 표면 품질이 망가진 후보가 선별되는 것을 막는다.
    const float HardPenalty = FMath::Max(1.0f, S.HardConstraintPenaltyWeight);
    const float RoughExcess = FMath::Max(0.f, M.RoughnessRatio - S.Targets.RoughnessMax);
    const float ShadowExcess = FMath::Max(0.f, M.ShadowRiskRatio - S.Targets.ShadowRiskMax);

    Score += (RoughExcess + ShadowExcess * 1.5f) * HardPenalty;

    // 표면을 거의 찾지 못한 후보는 Proxy가 불안정하므로 강하게 탈락시킨다.
    if (M.SurfaceNearSamples <= 3)
    {
        Score += HardPenalty * 10.f;
    }

    bOutPassed =
        InRange(M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax) &&
        InRange(M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax) &&
        (M.RoughnessRatio <= S.Targets.RoughnessMax) &&
        (M.ShadowRiskRatio <= S.Targets.ShadowRiskMax) &&
        (M.SurfaceNearSamples > 3);

    return Score;
}

static FMountainGenSettings MGMakeProxySettings(const FMountainGenSettings& S)
{
    FMountainGenSettings Proxy = S;
    Proxy.MetricsSamplesPerTry = FMath::Clamp(S.ProxyMetricsSamplesPerTry, 16, 2048);
    return Proxy;
}

static void MGApplyGoalFeedbackToSettings(FMountainGenSettings& InOutS, const FMGMetrics& M)
{
    const float TargetOver = RangeMid(InOutS.Targets.OverhangMin, InOutS.Targets.OverhangMax);
    const float TargetSteep = RangeMid(InOutS.Targets.SteepMin, InOutS.Targets.SteepMax);

    const float eOver = TargetOver - M.OverhangRatio;
    const float eSteep = TargetSteep - M.SteepRatio;
    const float eRough = FMath::Max(0.f, M.RoughnessRatio - InOutS.Targets.RoughnessMax);
    const float eShadow = FMath::Max(0.f, M.ShadowRiskRatio - InOutS.Targets.ShadowRiskMax);

    // 목표 오차 방향으로 다음 후보 그룹의 파라미터를 이동시킨다.
    // 이 단계는 최종 메시를 만들기 전 Proxy 탐색 안에서만 동작한다.
    InOutS.VolumeStrength += eOver * 0.30f;
    InOutS.OverhangDepthCm += eOver * 1600.f;
    InOutS.OverhangBias -= eOver * 0.08f;
    InOutS.OverhangFadeCm += eOver * 2400.f;

    InOutS.BaseField3DStrengthCm += eSteep * 3500.f;
    InOutS.BaseField3DScaleCm -= eSteep * 900.f;

    if (eRough > 0.f || eShadow > 0.f)
    {
        const float QualityError = eRough + eShadow * 1.75f;
        InOutS.DetailStrengthCm -= QualityError * 1100.f;
        InOutS.SurfaceRoughnessStrengthCm -= QualityError * 850.f;
        InOutS.SurfaceRoughnessMaskStrength += QualityError * 0.35f;
        InOutS.DetailScaleCm += QualityError * 1800.f;
    }

    MGClampToDifficultyBounds(InOutS);
}

static FString MGMakeSearchDebugLine(const TCHAR* Prefix, int32 Attempt, int32 MaxTry, const FMGSeedCandidate& C)
{
    const FMGMetrics& M = C.ProxyMetrics;
    return FString::Printf(
        TEXT("[%s] %d/%d seed=%d | ProxyScore=%.3f | Over=%.3f | Steep=%.3f | Rough=%.3f | Shadow=%.3f | Near=%d"),
        Prefix,
        Attempt,
        MaxTry,
        C.Seed,
        C.ProxyScore,
        M.OverhangRatio,
        M.SteepRatio,
        M.RoughnessRatio,
        M.ShadowRiskRatio,
        M.SurfaceNearSamples
    );
}

// Seed Search
// 목표 기반 계층 탐색:
// 1) 저해상도 Proxy Metrics로 많은 후보를 빠르게 평가한다.
// 2) 목표와 먼 후보는 메시 생성/정밀 평가 전에 버린다.
// 3) Proxy 상위 후보만 정밀 Metrics로 재평가한다.
// 4) Proxy 오차를 보고 다음 후보 그룹의 파라미터를 보정한다.
// 5) 선택된 후보의 Seed와 보정된 Settings를 InOutS에 반영한다.
// ============================================================

int32 MGSearchSeedForTargets(
    FMountainGenSettings& InOutS,
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
    const int32 LegacyTryBudget = bRetryUntilSatisfied ? FMath::Max(1, MaxAttempts) : FMath::Max(1, Tries);
    const int32 ProxyBudget = InOutS.bUseHierarchicalGoalSearch
        ? FMath::Max(1, InOutS.ProxySeedBudget)
        : LegacyTryBudget;

    const int32 TotalProxyBudget = FMath::Max(1, ProxyBudget);
    const int32 SurvivorCount = FMath::Clamp(InOutS.ProxySurvivorCount, 1, 128);
    const int32 FeedbackRounds = InOutS.bUseHierarchicalGoalSearch
        ? FMath::Clamp(InOutS.GoalFeedbackRounds, 1, 16)
        : 1;

    const int32 AutoBatch = FMath::Max(1, FMath::CeilToInt((float)TotalProxyBudget / (float)FeedbackRounds));
    const int32 BatchSize = InOutS.GoalFeedbackBatchSize > 0
        ? FMath::Clamp(InOutS.GoalFeedbackBatchSize, 1, 1024)
        : AutoBatch;

    FRandomStream Rng;
    if (InputSeed > 0) Rng.Initialize(InputSeed ^ 0x1F3A9B2D);
    else Rng.Initialize((int32)FPlatformTime::Cycles64());

    TArray<FMGSeedCandidate> Survivors;
    Survivors.Reserve(SurvivorCount * 2);

    FMountainGenSettings SearchBase = InOutS;
    MGClampToDifficultyBounds(SearchBase);

    FMGSeedCandidate BestProxy;
    FMGSeedCandidate BestFull;

    int32 Attempt = 0;

    for (int32 Round = 0; Round < FeedbackRounds && Attempt < TotalProxyBudget; ++Round)
    {
        FMGSeedCandidate BestRound;

        const int32 RoundCount = FMath::Min(BatchSize, TotalProxyBudget - Attempt);
        for (int32 i = 0; i < RoundCount; ++i)
        {
            ++Attempt;

            const int32 Seed = (Attempt == 1 && InputSeed > 0)
                ? InputSeed
                : Rng.RandRange(1, INT32_MAX);

            FMGSeedCandidate C;
            C.Seed = Seed;
            C.Settings = SearchBase;
            C.Settings.Seed = Seed;
            MGDeriveReproducibleDomainFromSeed(C.Settings, Seed);

            FMountainGenSettings ProxyS = MGMakeProxySettings(C.Settings);
            FMGMetricsContext ProxyCtx;
            ProxyCtx.Build(ProxyS, TerrainOriginWorld, WorldMin, WorldMax);
            C.ProxyMetrics = ProxyCtx.Compute(ProxyS);
            C.ProxyScore = MGComputeGoalScore(ProxyS, C.ProxyMetrics, C.bProxyPassed);

            if (C.ProxyScore < BestProxy.ProxyScore)
            {
                BestProxy = C;
            }
            if (C.ProxyScore < BestRound.ProxyScore)
            {
                BestRound = C;
            }

            Survivors.Add(C);
            Survivors.Sort([](const FMGSeedCandidate& A, const FMGSeedCandidate& B)
                {
                    return A.ProxyScore < B.ProxyScore;
                });
            if (Survivors.Num() > SurvivorCount)
            {
                Survivors.SetNum(SurvivorCount);
            }

            if (bDebug && DebugPrint)
            {
                const int32 EveryN = FMath::Max(1, DebugEveryN);
                if (Attempt == 1 || Attempt == TotalProxyBudget || (Attempt % EveryN) == 0)
                {
                    DebugPrint(
                        MGMakeSearchDebugLine(TEXT("ProxySearch"), Attempt, TotalProxyBudget, C),
                        3.0f,
                        C.bProxyPassed ? FColor::Green : FColor::Cyan
                    );
                }
            }
        }

        // Proxy 단계에서 목표를 만족한 후보가 나와도 바로 반환하지 않는다.
        // 반드시 정밀 Metrics 단계로 넘겨서 실제 선택 후보와 비교한다.
        if (InOutS.bUseHierarchicalGoalSearch && BestRound.ProxyScore < FLT_MAX)
        {
            MGApplyGoalFeedbackToSettings(SearchBase, BestRound.ProxyMetrics);
        }
    }

    if (Survivors.Num() == 0)
    {
        InOutS.Seed = (InputSeed > 0) ? InputSeed : 1;
        return InOutS.Seed;
    }

    // Proxy 상위 후보만 정밀 Metrics로 평가한다.
    // 여기서도 아직 메시를 만들지는 않는다. 밀도장 기반 정밀 지표로 마지막 Seed를 고른다.
    for (FMGSeedCandidate& C : Survivors)
    {
        FMountainGenSettings FullS = C.Settings;
        FullS.MetricsSamplesPerTry = FMath::Max(FullS.MetricsSamplesPerTry, InOutS.MetricsSamplesPerTry);

        FMGMetricsContext FullCtx;
        FullCtx.Build(FullS, TerrainOriginWorld, WorldMin, WorldMax);
        C.FullMetrics = FullCtx.Compute(FullS);
        C.FullScore = MGComputeGoalScore(FullS, C.FullMetrics, C.bFullPassed);

        if (C.FullScore < BestFull.FullScore)
        {
            BestFull = C;
        }

        if (bDebug && DebugPrint)
        {
            DebugPrint(
                FString::Printf(
                    TEXT("[RefineSearch] seed=%d | FullScore=%.3f | Over=%.3f | Steep=%.3f | Rough=%.3f | Shadow=%.3f | Near=%d"),
                    C.Seed,
                    C.FullScore,
                    C.FullMetrics.OverhangRatio,
                    C.FullMetrics.SteepRatio,
                    C.FullMetrics.RoughnessRatio,
                    C.FullMetrics.ShadowRiskRatio,
                    C.FullMetrics.SurfaceNearSamples),
                4.5f,
                C.bFullPassed ? FColor::Green : FColor::Orange
            );
        }

        // 정밀 평가에서 완전 통과한 후보가 있으면 점수순으로 이미 비교되므로,
        // 더 낮은 점수를 찾기 위해 끝까지 본다. 조기 반환하지 않는다.
    }

    if (BestFull.FullScore < FLT_MAX)
    {
        InOutS = BestFull.Settings;
        InOutS.Seed = BestFull.Seed;
        MGClampToDifficultyBounds(InOutS);
        MGDeriveReproducibleDomainFromSeed(InOutS, BestFull.Seed);

        if (bDebug && DebugPrint)
        {
            DebugPrint(
                FString::Printf(
                    TEXT("[GoalSearch][Selected] seed=%d | Score=%.3f | ProxyScore=%.3f | Passed=%s"),
                    BestFull.Seed,
                    BestFull.FullScore,
                    BestFull.ProxyScore,
                    BestFull.bFullPassed ? TEXT("true") : TEXT("false")),
                6.0f,
                BestFull.bFullPassed ? FColor::Green : FColor::Yellow
            );
        }

        return BestFull.Seed;
    }

    InOutS = BestProxy.Settings;
    InOutS.Seed = BestProxy.Seed;
    MGClampToDifficultyBounds(InOutS);
    MGDeriveReproducibleDomainFromSeed(InOutS, BestProxy.Seed);
    return BestProxy.Seed;
}

void MGDeriveReproducibleDomainFromSeed(FMountainGenSettings& InOutS, int32 Seed)
{
    InOutS.Seed = FMath::Max(1, Seed);
}