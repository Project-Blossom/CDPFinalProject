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

static bool FindIsoCrossingAlongX(
    const FVoxelDensityGenerator& Gen,
    float Iso,
    const FVector& StartOutside,
    const FVector& EndInside,
    int32 Steps,
    FVector& OutHit)
{
    float d0 = Gen.SampleDensity(StartOutside) - Iso;
    float d1 = Gen.SampleDensity(EndInside) - Iso;

    if (d0 * d1 > 0.f) return false;

    FVector a = StartOutside;
    FVector b = EndInside;

    Steps = FMath::Clamp(Steps, 6, 24);
    for (int32 i = 0; i < Steps; i++)
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
// Metrics: RaycastYZ
// ============================================================

FMGMetrics MGComputeMetrics_RaycastYZ(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    FMGMetrics M{};

    const FVoxelDensityGenerator Gen(S, TerrainOriginWorld);
    const float Iso = S.IsoLevel;

    const int32 TotalRays = FMath::Clamp(S.MetricsSamplesPerTry, 16, 4096);
    const int32 Grid = FMath::Max(4, (int32)FMath::Sqrt((float)TotalRays));
    const int32 Ny = Grid;
    const int32 Nz = Grid;

    const float y0 = WorldMin.Y;
    const float y1 = WorldMax.Y;
    const float z0 = WorldMin.Z;
    const float z1 = WorldMax.Z;

    const float FrontX = FMath::Max(200.f, S.CliffThicknessCm);
    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    const float SurfaceBand = FMath::Max(Voxel * 3.f, FMath::Min(S.OverhangFadeCm, 6000.f));

    const float SteepDot = GetSteepDotThreshold(S);

    int32 Valid = 0;
    int32 Over = 0;
    int32 Steep = 0;

    const FVector Up(0, 0, 1);
    const float e = FMath::Max(5.f, Voxel * 0.25f);

    auto MakeRayEndpoints = [&](float y, float z, FVector& OutOutside, FVector& OutInside)
        {
            const FVector A(WorldMin.X, y, z);
            const FVector B(WorldMax.X, y, z);

            const float da = Gen.SampleDensity(A);
            const float db = Gen.SampleDensity(B);

            if (da < Iso && db >= Iso) { OutOutside = A; OutInside = B; return; }
            if (db < Iso && da >= Iso) { OutOutside = B; OutInside = A; return; }

            OutOutside = A;
            OutInside = B;
        };

    static constexpr float BaseCliff_NxAbs_Threshold = 0.97f;
    static constexpr float BaseCliff_UpAbs_Threshold = 0.12f;
    static constexpr float BaseCliff_NyAbs_Threshold = 0.25f;

    for (int32 iz = 0; iz < Nz; ++iz)
    {
        const float z = FMath::Lerp(z0, z1, (iz + 0.5f) / (float)Nz);

        for (int32 iy = 0; iy < Ny; ++iy)
        {
            const float y = FMath::Lerp(y0, y1, (iy + 0.5f) / (float)Ny);

            FVector Outside, Inside;
            MakeRayEndpoints(y, z, Outside, Inside);

            FVector Hit;
            if (!FindIsoCrossingAlongX(Gen, Iso, Outside, Inside, 14, Hit))
                continue;

            const float LocalX = Hit.X - TerrainOriginWorld.X;
            const float insideDepth = (FrontX - LocalX);

            if (insideDepth < 0.f || insideDepth > SurfaceBand)
                continue;

            const FVector N = EstimateNormalCentralDiff(Gen, Hit, e);
            if (N.IsNearlyZero())
                continue;

            const float UpDot = FVector::DotProduct(N, Up);

            const float NxAbs = FMath::Abs(N.X);
            const float NyAbs = FMath::Abs(N.Y);
            const bool bIsBaseCliff =
                (NxAbs >= BaseCliff_NxAbs_Threshold) &&
                (FMath::Abs(UpDot) <= BaseCliff_UpAbs_Threshold) &&
                (NyAbs <= BaseCliff_NyAbs_Threshold);

            if (bIsBaseCliff)
                continue;

            Valid++;

            static constexpr float OverhangUpDotThreshold = -0.15f;

            if (UpDot <= OverhangUpDotThreshold)
            {
                Over++;
            }
            if (FMath::Abs(UpDot) <= SteepDot) Steep++;
        }
    }

    M.SurfaceNearSamples = Valid;
    if (Valid > 0)
    {
        M.OverhangRatio = (float)Over / (float)Valid;
        M.SteepRatio = (float)Steep / (float)Valid;
    }
    else
    {
        M.OverhangRatio = 0.f;
        M.SteepRatio = 0.f;
    }

    return M;
}

// ============================================================================
// Difficulty Preset
// ============================================================================

FMGDifficultyPreset MGMakeDifficultyPreset(EMountainGenDifficulty D)
{
    // ✅ Extreme은 없앤다: 내부적으로 Hard로 동작
    // (enum/블루프린트 레퍼런스 안 깨지게 유지)
    if (D == EMountainGenDifficulty::Extreme)
    {
        D = EMountainGenDifficulty::Hard;
    }

    FMGDifficultyPreset P;
    auto& B = P.Bounds;

    switch (D)
    {
    case EMountainGenDifficulty::Easy:
        P.Targets.OverhangMin = 0.12f; P.Targets.OverhangMax = 0.38f;
        P.Targets.SteepMin = 0.15f; P.Targets.SteepMax = 0.35f;

        P.BaseField3DStrengthCm = 6500.f;
        P.BaseField3DScaleCm = 22000.f;
        P.DetailScaleCm = 8000.f;

        P.VolumeStrength = 0.35f;
        P.OverhangScaleCm = 14000.f;
        P.OverhangFadeCm = 24000.f;
        P.OverhangBias = 0.72f;
        P.OverhangDepthCm = 2200.f;

        P.GravityStrength = 1.20f;
        P.GravityScale = 2.3f;

        B.BaseField3DStrengthCm = { 3000.f, 9000.f };
        B.BaseField3DScaleCm = { 18000.f, 30000.f };
        B.DetailScaleCm = { 6500.f, 14000.f };

        B.VolumeStrength = { 0.10f, 0.55f };
        B.OverhangScaleCm = { 10000.f, 20000.f };
        B.OverhangBias = { 0.66f, 0.82f };
        B.OverhangDepthCm = { 800.f, 3200.f };
        B.OverhangFadeCm = { 18000.f, 32000.f };

        B.GravityStrength = { 1.05f, 1.45f };
        B.GravityScale = { 2.0f, 3.0f };
        break;

    case EMountainGenDifficulty::Normal:
        P.Targets.OverhangMin = 0.18f; P.Targets.OverhangMax = 0.45f;
        P.Targets.SteepMin = 0.18f; P.Targets.SteepMax = 0.40f;

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
    default:
        P.Targets.OverhangMin = 0.28f; P.Targets.OverhangMax = 0.60f;
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

// ============================================================
// Metrics (외부 이름 유지: MGComputeMetricsQuick)
// - 내부 구현을 RaycastYZ로 교체 (진짜 언더컷 측정)
// ============================================================

FMGMetrics MGComputeMetricsQuick(
    const FMountainGenSettings& S,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMin,
    const FVector& WorldMax)
{
    // ✅ 기존 호출부는 그대로, 측정 품질만 교체
    return MGComputeMetrics_RaycastYZ(S, TerrainOriginWorld, WorldMin, WorldMax);
}

// ============================================================
// AutoTune (intent params)
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

// ============================================================
// Seed Search
// ============================================================

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

        if (bOverOK && bSteepOK)
        {
            if (bDebug && DebugPrint)
            {
                const FString Line = FString::Printf(
                    TEXT("seed=%d | Over=%.3f [%.2f~%.2f] %s | Steep=%.3f [%.2f~%.2f] %s | Near=%d | Score=%.3f"),
                    Seed,
                    M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax, TEXT("OK"),
                    M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax, TEXT("OK"),
                    M.SurfaceNearSamples,
                    Score
                );

                DebugPrint(
                    FString::Printf(
                        TEXT("[SeedSearch] %d/%d SUCCESS %s"),
                        Attempt, MaxTry, *Line
                    ),
                    6.0f,
                    FColor::Green
                );
            }

            return Seed;
        }
    }

    return BestSeed;
}

void MGDeriveReproducibleDomainFromSeed(FMountainGenSettings& InOutS, int32 Seed)
{
    InOutS.Seed = FMath::Max(1, Seed);
}