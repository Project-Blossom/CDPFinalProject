#include "VoxelDensityGenerator.h"

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

float FVoxelDensityGenerator::FBM3D(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const
{
    Octaves = FMath::Clamp(Octaves, 1, 12);

    float sum = 0.f;
    float amp = 1.f;
    float freq = 1.f;

    for (int32 i = 0; i < Octaves; ++i)
    {
        sum += Noise3D(p * freq) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return sum;
}

float FVoxelDensityGenerator::RidgedFBM01(const FVector& p, int32 Octaves, float Lacunarity, float Gain) const
{
    Octaves = FMath::Clamp(Octaves, 1, 12);

    float sum = 0.f;
    float amp = 0.5f;
    float freq = 1.f;

    for (int32 i = 0; i < Octaves; ++i)
    {
        const float n = Noise3D(p * freq);
        sum += Ridged01(n) * amp;
        freq *= Lacunarity;
        amp *= Gain;
    }
    return Clamp01(sum);
}


float FVoxelDensityGenerator::PlateauFBM2D(const FVector2D& p, int32 Octaves, float Lacunarity, float Gain) const
{
    Octaves = FMath::Clamp(Octaves, 1, 8);

    float Sum = 0.f;
    float Amp = 0.5f;
    float Freq = 1.f;
    float Norm = 0.f;

    for (int32 i = 0; i < Octaves; ++i)
    {
        Sum += FMath::PerlinNoise2D(p * Freq) * Amp;
        Norm += Amp;
        Amp *= Gain;
        Freq *= Lacunarity;
    }

    return (Norm > KINDA_SMALL_NUMBER) ? (Sum / Norm) : 0.f;
}

float FVoxelDensityGenerator::ComputeTopPlateauSurfaceZ(const FVector& LocalCm) const
{
    const float ActorX = LocalCm.X - C.FrontX;
    const float ActorY = LocalCm.Y;

    const float CliffTopZ = S.BaseHeightCm + C.CliffH - FMath::Max(0.f, S.TopPlateauContactOverlapDownCm);
    const float TopBaseZ = S.BaseHeightCm + C.CliffH + S.TopPlateauHeightOffsetCm;

    const float Strength = FMath::Max(0.f, S.PlateauSurfaceNoiseStrengthCm);
    const float Depth = FMath::Max(C.Voxel * 5.f, S.TopPlateauDepthCm);

    const float DistanceBehindContact = FMath::Max(0.f, -ActorX);
    const float HillT = SmoothStep01(DistanceBehindContact / Depth);

    const float FadeCm = FMath::Max(0.f, S.PlateauSurfaceEdgeFadeCm);
    float EdgeAlpha = 1.f;
    if (FadeCm > KINDA_SMALL_NUMBER)
    {
        const float HalfW = FMath::Max(10.f, S.CliffHalfWidthCm);
        const float DistLeft = FMath::Max(0.f, ActorY + HalfW);
        const float DistRight = FMath::Max(0.f, HalfW - ActorY);
        const float EdgeDist = FMath::Min(DistLeft, DistRight);
        EdgeAlpha = SmoothStep01(EdgeDist / FadeCm);
    }

    float ReliefCm = 0.f;
    if (Strength > KINDA_SMALL_NUMBER)
    {
        const float Scale = FMath::Max(C.Voxel * 8.f, S.PlateauSurfaceNoiseScaleCm * 0.70f);
        const int32 Octaves = FMath::Clamp(S.PlateauSurfaceNoiseOctaves, 1, 8);
        const int32 PlateauSeed = FMath::Max(1, Seed + S.PlateauSeedOffset);
        const FVector2D SeedShift((float)(PlateauSeed % 7919) * 0.017f, (float)(PlateauSeed % 3571) * 0.023f);

        const FVector2D BaseP(ActorX / Scale, ActorY / Scale);
        const FVector2D WarpP(ActorX / (Scale * 1.85f), ActorY / (Scale * 1.85f));

        const float WarpX = PlateauFBM2D(WarpP + SeedShift + FVector2D(17.2f, 3.1f), 3, 2.02f, 0.50f);
        const float WarpY = PlateauFBM2D(WarpP + SeedShift + FVector2D(-5.4f, 11.7f), 3, 2.02f, 0.50f);
        const FVector2D P = BaseP + FVector2D(WarpX, WarpY) * 0.40f;

        const float Large = PlateauFBM2D(P * 0.45f + SeedShift + FVector2D(4.7f, -8.2f), FMath::Max(1, Octaves - 2), 2.0f, 0.55f);
        const float Rolling = PlateauFBM2D(P * 0.90f + SeedShift + FVector2D(31.4f, -9.8f), FMath::Max(1, Octaves - 1), 2.02f, 0.52f);
        const float Mid = PlateauFBM2D(P * 1.75f + SeedShift + FVector2D(-12.6f, 44.3f), Octaves, 2.05f, 0.48f);
        const float Detail = PlateauFBM2D(P * 3.25f + SeedShift + FVector2D(71.0f, 19.0f), FMath::Max(1, Octaves - 2), 2.10f, 0.45f);

        const float RidgeBase = PlateauFBM2D(P * 1.20f + SeedShift + FVector2D(-44.0f, 23.0f), FMath::Max(1, Octaves - 1), 2.05f, 0.50f);
        const float Ridge = 1.f - FMath::Abs(RidgeBase);
        const float RidgeSigned = Ridge * 2.f - 1.f;

        const float SignedHill = FMath::Clamp(
            Large * 0.48f +
            Rolling * 0.36f +
            Mid * 0.24f +
            Detail * 0.08f +
            RidgeSigned * 0.18f,
            -1.f,
            1.f);

        ReliefCm = SignedHill * Strength * 0.65f * HillT;
    }

    const float BaseHillRiseCm = FMath::Max(0.f, TopBaseZ - CliffTopZ) + Strength * 0.55f;
    const float HillRiseCm = FMath::Max(0.f, BaseHillRiseCm * HillT + ReliefCm);

    return CliffTopZ + HillRiseCm * EdgeAlpha;
}

float FVoxelDensityGenerator::ApplyIntegratedTopPlateauCap(float Density, const FVector& LocalCm) const
{
    return Density;
}


bool FVoxelDensityGenerator::TryComputeTopPlateauSurfaceNormal(
    const FVector& WorldPosCm,
    float ToleranceCm,
    FVector& OutNormal,
    float& OutWeight) const
{
    OutNormal = FVector::ZeroVector;
    OutWeight = 0.f;

    if (!S.bAddTopFlatPlateau)
    {
        return false;
    }

    const FVector Local = WorldPosCm - TerrainOriginWorld;
    const float ActorX = Local.X - C.FrontX;
    const float ActorY = Local.Y;

    const float Depth = FMath::Max(0.f, S.TopPlateauDepthCm);
    if (Depth <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    const float HalfW = FMath::Max(10.f, S.CliffHalfWidthCm);
    const float BackPaddingCm = C.Voxel * 4.f;

    const float ContactForwardBlendCm = FMath::Max(
        C.Voxel * 8.f,
        FMath::Max(S.PlateauSurfaceEdgeFadeCm * 1.50f, S.TopPlateauMaxConformDropCm * 0.35f));

    const bool bInsidePlateauDomain =
        ActorX <= ContactForwardBlendCm &&
        ActorX >= -(Depth + BackPaddingCm) &&
        FMath::Abs(ActorY) <= HalfW;

    if (!bInsidePlateauDomain)
    {
        return false;
    }

    const float CliffTopZForBase = S.BaseHeightCm + C.CliffH - FMath::Max(0.f, S.TopPlateauContactOverlapDownCm);

    const float TopCutZ = (ActorX <= 0.f)
        ? FMath::Max(CliffTopZForBase, ComputeTopPlateauSurfaceZ(Local))
        : CliffTopZForBase;

    const float Tol = FMath::Max(C.Voxel * 1.25f, ToleranceCm);

    const float SignedDistanceToTop = Local.Z - TopCutZ;
    const float DistanceToTopSurface = FMath::Abs(SignedDistanceToTop);
    const float BelowTopDepth = FMath::Max(0.f, -SignedDistanceToTop);

    const float ContactDownBlendDepth = FMath::Max(
        C.Voxel * 12.f,
        FMath::Max(S.PlateauSurfaceEdgeFadeCm * 1.75f, S.TopPlateauMaxConformDropCm * 0.85f));

    const bool bNearTopSurface = DistanceToTopSurface <= Tol;
    const bool bInsideContactShoulderBand =
        SignedDistanceToTop <= Tol &&
        BelowTopDepth <= ContactDownBlendDepth;

    const bool bInsideFrontLipBand =
        ActorX > 0.f &&
        ActorX <= ContactForwardBlendCm &&
        SignedDistanceToTop <= Tol &&
        BelowTopDepth <= ContactDownBlendDepth;

    if (!bNearTopSurface && !bInsideContactShoulderBand && !bInsideFrontLipBand)
    {
        return false;
    }

    const float Step = FMath::Max(C.Voxel, 100.f);

    FVector LXp = Local;
    FVector LXm = Local;
    FVector LYp = Local;
    FVector LYm = Local;

    LXp.X += Step;
    LXm.X -= Step;
    LYp.Y += Step;
    LYm.Y -= Step;

    const auto EvalTopZ = [this, CliffTopZForBase](const FVector& L) -> float
        {
            return FMath::Max(CliffTopZForBase, ComputeTopPlateauSurfaceZ(L));
        };

    const float DzDx = (EvalTopZ(LXp) - EvalTopZ(LXm)) / (Step * 2.f);
    const float DzDy = (EvalTopZ(LYp) - EvalTopZ(LYm)) / (Step * 2.f);

    FVector N(-DzDx, -DzDy, 1.f);
    if (!N.Normalize())
    {
        N = FVector::UpVector;
    }

    if (N.Z < 0.f)
    {
        N *= -1.f;
    }

    const float SurfaceWeight = 1.f - SmoothStep01(DistanceToTopSurface / Tol);

    const float ShoulderWeight = 1.f - SmoothStep01(BelowTopDepth / ContactDownBlendDepth);

    const float ContactBlendLen = FMath::Max(C.Voxel * 8.f, S.PlateauSurfaceEdgeFadeCm * 1.25f);
    const float ContactT = (ActorX <= 0.f)
        ? SmoothStep01(FMath::Clamp((-ActorX) / ContactBlendLen, 0.f, 1.f))
        : 0.f;

    const float FrontLipT = (ActorX > 0.f)
        ? (1.f - SmoothStep01(FMath::Clamp(ActorX / ContactForwardBlendCm, 0.f, 1.f)))
        : 0.f;

    const float ContactWeight = (ActorX <= 0.f)
        ? FMath::Lerp(0.70f, 1.f, ContactT)
        : FMath::Lerp(0.55f, 0.90f, FrontLipT);

    const float RawWeight = FMath::Max(SurfaceWeight, ShoulderWeight * (ActorX > 0.f ? 0.95f : 0.75f));

    OutNormal = N;
    OutWeight = FMath::Clamp(RawWeight * ContactWeight, 0.f, 1.f);

    return OutWeight > KINDA_SMALL_NUMBER;
}

void FVoxelDensityGenerator::InitSeedDomain()
{
    FRandomStream Rng(Seed ^ 0x6C8E9CF5);

    const float OffsetRangeCm = 800000.0f;
    SeedOffsetCm = FVector(
        Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm),
        Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm),
        Rng.FRandRange(-OffsetRangeCm, OffsetRangeCm)
    );

    const float RandomnessCap = 0.35f;
    const float Randomness = FMath::Min(Rng.FRand(), RandomnessCap);

    const float MulMin = FMath::Lerp(0.98f, 0.93f, Randomness);
    const float MulMax = FMath::Lerp(1.02f, 1.07f, Randomness);
    SeedScaleMul = Rng.FRandRange(MulMin, MulMax);

    const float PKeep = 0.85f;
    AxisShuffle = (Rng.FRand() < PKeep) ? 0 : Rng.RandRange(0, 5);
}

void FVoxelDensityGenerator::InitCachedConstants()
{
    C.Iso = S.IsoLevel;
    C.Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    C.BaseFieldAmp = FMath::Clamp(S.BaseField3DStrengthCm, 0.f, 50000.f);

    C.CliffH = FMath::Max(1000.f, S.CliffHeightCm);
    C.FrontX = FMath::Max(200.f, S.CliffThicknessCm);

    // Overhang
    C.OverhangBand = FMath::Max(C.Voxel * 3.f, FMath::Min(S.OverhangFadeCm, 6000.f));
    C.OverhangScale = FMath::Max(200.f, S.OverhangScaleCm);
    C.OverhangBias = FMath::Clamp(S.OverhangBias, 0.0f, 1.0f);
    C.OverhangAmp = FMath::Max(0.f, S.VolumeStrength) * FMath::Max(0.f, S.OverhangDepthCm);

    // Macro 3D
    C.Base3DScale = FMath::Max(2000.f, S.BaseField3DScaleCm);
    C.Base3DOct = FMath::Clamp(S.BaseField3DOctaves, 1, 8);

    // Detail
    C.DetailScale = FMath::Max(C.Voxel * 4.f, S.DetailScaleCm);
    C.DetailAmp = FMath::Clamp(S.DetailStrengthCm, 0.f, 12000.f);
    C.DetailOct = FMath::Clamp(S.DetailOctaves, 1, 4);

    // Roughness near surface
    C.RoughBand = FMath::Max(150.f, C.Voxel * 3.f);
    C.RoughScale = FMath::Max(C.Voxel * 3.f, C.DetailScale * 0.45f);
    C.RoughAmp = FMath::Clamp(S.SurfaceRoughnessStrengthCm, 0.f, 8000.f);
    C.RoughMaskStrength = FMath::Clamp(S.SurfaceRoughnessMaskStrength, 0.f, 1.f);
}

float FVoxelDensityGenerator::SampleDensity(const FVector& WorldPosCm) const
{
    const float Iso = C.Iso;
    const FVector Local = WorldPosCm - TerrainOriginWorld;
    const FVector Domain = SeededDomain(Local);
    const float Voxel = C.Voxel;

    const float BaseFieldAmp = C.BaseFieldAmp;

    // -----------------------------
    // Cliff 기반 면
    // -----------------------------
    const float CliffH = C.CliffH;
    const float FrontX = C.FrontX;

    const float z01 = Clamp01((Local.Z - S.BaseHeightCm) / CliffH);

    float density = (FrontX - Local.X);

    // -------------------------------------------------
    // 3) Overhang / Undercut field
    // -------------------------------------------------
    {
        const float insideDepth = (FrontX - Local.X);

        const float band = C.OverhangBand;

        const float d = FMath::Max(0.f, insideDepth);
        const float nearFaceMask = 1.f - SmoothStep(0.f, band, d);

        const float mid = 1.f - FMath::Abs(2.f * z01 - 1.f);
        const float heightMask = FMath::Pow(FMath::Clamp(mid, 0.f, 1.f), 1.6f);

        float Amp = C.OverhangAmp;
        if (S.TerrainAlgorithm == EMGTerrainAlgorithm::DensityFBM)
        {
            Amp *= 0.35f;
        }
        else if (S.TerrainAlgorithm == EMGTerrainAlgorithm::LayeredNoise)
        {
            Amp *= 0.70f;
        }
        else if (S.TerrainAlgorithm == EMGTerrainAlgorithm::ZoneMaskedDensity)
        {
            const float TopBoost = SmoothStep(0.35f, 0.90f, z01);
            Amp *= FMath::Lerp(0.55f, 1.25f, TopBoost);
        }

        if (Amp != 0.f && nearFaceMask != 0.f && heightMask != 0.f)
        {
            const float Scale = C.OverhangScale;
            const float r = RidgedFBM01(Domain / Scale, 5, 2.0f, 0.55f); // 0..1

            const float bias = C.OverhangBias;
            const float shaped = (r - bias);

            density += shaped * Amp * nearFaceMask * heightMask;
        }
    }

    // -----------------------------
    // Macro 3D
    // -----------------------------
    {
        if (BaseFieldAmp != 0.f)
        {
            const float Scale = C.Base3DScale;
            const int32 Oct = C.Base3DOct;

            float n = FBM3D(Domain / Scale, Oct, 2.0f, 0.5f);

            if (S.TerrainAlgorithm == EMGTerrainAlgorithm::LayeredNoise)
            {
                const float Layer = FBM3D((Domain + FVector(9131.f, -2217.f, 5411.f)) / (Scale * 0.42f), FMath::Max(1, Oct - 1), 2.15f, 0.48f);
                n = n * 0.72f + Layer * 0.28f;
            }
            else if (S.TerrainAlgorithm == EMGTerrainAlgorithm::ZoneMaskedDensity)
            {
                const float HeightMask = FMath::Lerp(0.75f, 1.20f, SmoothStep(0.20f, 0.85f, z01));
                n *= HeightMask;
            }

            density += n * BaseFieldAmp;
        }
    }

    // -----------------------------
    // Detail
    // -----------------------------
    {
        if (C.DetailAmp != 0.f)
        {
            const float Scale = C.DetailScale;
            const int32 Oct = C.DetailOct;

            float n = FBM3D(Domain / Scale, Oct, 2.0f, 0.55f);
            float DetailAmp = C.DetailAmp;

            if (S.TerrainAlgorithm == EMGTerrainAlgorithm::DensityFBM)
            {
                DetailAmp *= 0.55f;
            }
            else if (S.TerrainAlgorithm == EMGTerrainAlgorithm::LayeredNoise)
            {
                n = n * 0.70f + RidgedFBM01((Domain + FVector(-3711.f, 7187.f, 1297.f)) / (Scale * 0.75f), Oct, 2.0f, 0.50f) * 0.30f;
                DetailAmp *= 1.15f;
            }

            density += n * DetailAmp;
        }
    }

    // -----------------------------
    // Integrated top terrain cap
    // -----------------------------
    if (S.bAddTopFlatPlateau)
    {
        const float CliffTopZForBase = S.BaseHeightCm + C.CliffH - FMath::Max(0.f, S.TopPlateauContactOverlapDownCm);
        const float ActorXForPlateau = Local.X - C.FrontX;
        const float Depth = FMath::Max(0.f, S.TopPlateauDepthCm);
        const float HalfW = FMath::Max(10.f, S.CliffHalfWidthCm);

        const float BackPaddingCm = C.Voxel * 4.f;
        const bool bInsidePlateauHillDomain =
            Depth > KINDA_SMALL_NUMBER &&
            ActorXForPlateau <= 0.f &&
            ActorXForPlateau >= -(Depth + BackPaddingCm) &&
            FMath::Abs(Local.Y) <= HalfW;

        const float TopCutZ = bInsidePlateauHillDomain
            ? FMath::Max(CliffTopZForBase, ComputeTopPlateauSurfaceZ(Local))
            : CliffTopZForBase;

        density = FMath::Min(density, TopCutZ - Local.Z);
    }

    density = ApplyIntegratedTopPlateauCap(density, Local);

    const float ActorXForTop = Local.X - C.FrontX;
    const bool bInIntegratedTopDomain =
        S.bAddTopFlatPlateau &&
        ActorXForTop <= 0.f &&
        Local.Z >= S.BaseHeightCm + C.CliffH - FMath::Max(C.RoughBand, Voxel * 4.f);

    // -----------------------------
    //  추가 거칠기
    // -----------------------------
    {
        const float Band = C.RoughBand;
        const float surfMask = 1.f - SmoothStep(0.f, Band, FMath::Abs(density - Iso));

        if (surfMask > 0.f && C.RoughAmp != 0.f)
        {
            const float Scale = C.RoughScale;
            const float n = FBM3D(Domain / Scale, 3, 2.0f, 0.55f);

            const float TopRoughnessScale = bInIntegratedTopDomain ? 0.35f : 1.f;

            const float SafeMask = FMath::Pow(surfMask, 1.0f + C.RoughMaskStrength * 2.0f);
            density += n * C.RoughAmp * SafeMask * TopRoughnessScale;
        }
    }

    return density;
}