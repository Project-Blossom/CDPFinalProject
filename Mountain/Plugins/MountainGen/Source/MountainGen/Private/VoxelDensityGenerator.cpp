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

    float Hill = 0.f;

    if (Strength > KINDA_SMALL_NUMBER)
    {
        const float Scale = FMath::Max(1000.f, S.PlateauSurfaceNoiseScaleCm);
        const int32 Octaves = FMath::Clamp(S.PlateauSurfaceNoiseOctaves, 1, 8);
        const int32 PlateauSeed = FMath::Max(1, Seed + S.PlateauSeedOffset);
        const FVector2D SeedShift((float)(PlateauSeed % 7919) * 0.017f, (float)(PlateauSeed % 3571) * 0.023f);

        FVector2D P(ActorX / Scale, ActorY / Scale);

        const float WarpScale = Scale * 1.85f;
        const FVector2D WP(ActorX / WarpScale, ActorY / WarpScale);
        const float WarpX = PlateauFBM2D(WP + SeedShift + FVector2D(17.2f, 3.1f), 3, 2.02f, 0.5f);
        const float WarpY = PlateauFBM2D(WP + SeedShift + FVector2D(-5.4f, 11.7f), 3, 2.02f, 0.5f);
        P += FVector2D(WarpX, WarpY) * 0.42f;

        const float Macro = PlateauFBM2D(P * 0.72f + SeedShift, FMath::Max(1, Octaves - 2), 2.02f, 0.5f);
        const float Mid = PlateauFBM2D(P * 1.55f + SeedShift + FVector2D(31.4f, -9.8f), FMath::Max(1, Octaves - 1), 2.02f, 0.5f);
        const float Micro = PlateauFBM2D(P * 4.25f + SeedShift + FVector2D(-12.6f, 44.3f), FMath::Max(1, Octaves - 2), 2.02f, 0.5f);

        Hill = Macro * 0.72f + Mid * 0.23f + Micro * 0.05f;
        Hill = (Hill + 1.f) * 0.5f;
        Hill = SmoothStep01(Hill);
        Hill = FMath::Pow(Hill, 1.35f);
    }

    const float FadeCm = FMath::Max(0.f, S.PlateauSurfaceEdgeFadeCm);
    float EdgeFade = 1.f;
    if (FadeCm > KINDA_SMALL_NUMBER)
    {
        const float XBack = -FMath::Max(0.f, S.TopPlateauDepthCm);
        const float HalfW = FMath::Max(10.f, S.CliffHalfWidthCm);
        const float DistBack = FMath::Max(0.f, ActorX - XBack);
        const float DistLeft = FMath::Max(0.f, ActorY + HalfW);
        const float DistRight = FMath::Max(0.f, HalfW - ActorY);
        const float EdgeDist = FMath::Min(DistBack, FMath::Min(DistLeft, DistRight));
        EdgeFade = SmoothStep01(EdgeDist / FadeCm);
    }

    const float NaturalTopZ = TopBaseZ + Hill * Strength;
    const float EdgeLimitedTopZ = FMath::Lerp(CliffTopZ, NaturalTopZ, EdgeFade);

    const float FrontTransitionLen = FMath::Clamp(
        FMath::Max(S.TopPlateauMaxConformDropCm, S.PlateauSurfaceEdgeFadeCm),
        C.Voxel * 4.f,
        8000.f);

    const float DistanceBehindFront = FMath::Max(0.f, -ActorX);
    const float FrontAlpha = SmoothStep01(DistanceBehindFront / FrontTransitionLen);

    return FMath::Lerp(CliffTopZ, EdgeLimitedTopZ, FrontAlpha);
}

float FVoxelDensityGenerator::ApplyIntegratedTopPlateauCap(float Density, const FVector& LocalCm) const
{
    if (!S.bAddTopFlatPlateau)
    {
        return Density;
    }

    const float ActorX = LocalCm.X - C.FrontX;
    const float ActorY = LocalCm.Y;

    const float Depth = FMath::Max(0.f, S.TopPlateauDepthCm);
    if (Depth <= KINDA_SMALL_NUMBER)
    {
        return Density;
    }

    const float HalfW = FMath::Max(10.f, S.CliffHalfWidthCm);
    if (ActorX > 0.f || ActorX < -Depth || FMath::Abs(ActorY) > HalfW)
    {
        return Density;
    }

    auto SampleBaseCliffNoTop = [this](const FVector& L) -> float
        {
            const FVector Domain = SeededDomain(L);
            const float z01 = Clamp01((L.Z - S.BaseHeightCm) / C.CliffH);

            float D = (C.FrontX - L.X);

            {
                const float insideDepth = (C.FrontX - L.X);
                const float d = FMath::Max(0.f, insideDepth);
                const float nearFaceMask = 1.f - SmoothStep(0.f, C.OverhangBand, d);

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
                    const float r = RidgedFBM01(Domain / C.OverhangScale, 5, 2.0f, 0.55f);
                    const float shaped = (r - C.OverhangBias);
                    D += shaped * Amp * nearFaceMask * heightMask;
                }
            }

            if (C.BaseFieldAmp != 0.f)
            {
                float n = FBM3D(Domain / C.Base3DScale, C.Base3DOct, 2.0f, 0.5f);

                if (S.TerrainAlgorithm == EMGTerrainAlgorithm::LayeredNoise)
                {
                    const float Layer = FBM3D((Domain + FVector(9131.f, -2217.f, 5411.f)) / (C.Base3DScale * 0.42f), FMath::Max(1, C.Base3DOct - 1), 2.15f, 0.48f);
                    n = n * 0.72f + Layer * 0.28f;
                }
                else if (S.TerrainAlgorithm == EMGTerrainAlgorithm::ZoneMaskedDensity)
                {
                    const float HeightMask = FMath::Lerp(0.75f, 1.20f, SmoothStep(0.20f, 0.85f, z01));
                    n *= HeightMask;
                }

                D += n * C.BaseFieldAmp;
            }

            if (C.DetailAmp != 0.f)
            {
                float n = FBM3D(Domain / C.DetailScale, C.DetailOct, 2.0f, 0.55f);
                float DetailAmp = C.DetailAmp;

                if (S.TerrainAlgorithm == EMGTerrainAlgorithm::DensityFBM)
                {
                    DetailAmp *= 0.55f;
                }
                else if (S.TerrainAlgorithm == EMGTerrainAlgorithm::LayeredNoise)
                {
                    n = n * 0.70f + RidgedFBM01((Domain + FVector(-3711.f, 7187.f, 1297.f)) / (C.DetailScale * 0.75f), C.DetailOct, 2.0f, 0.50f) * 0.30f;
                    DetailAmp *= 1.15f;
                }

                D += n * DetailAmp;
            }

            return D;
        };

    const float CliffTopZ = S.BaseHeightCm + C.CliffH - FMath::Max(0.f, S.TopPlateauContactOverlapDownCm);
    const float SupportZ = CliffTopZ - FMath::Max(C.Voxel * 2.f, 50.f);
    const float SupportDensity = SampleBaseCliffNoTop(FVector(LocalCm.X, LocalCm.Y, SupportZ));

    const float RequiredSupportDepth = FMath::Max(C.Voxel * 2.f, 200.f);
    const float SupportFade = FMath::Max(C.Voxel * 2.f, S.PlateauSurfaceEdgeFadeCm * 0.35f);
    const float SupportAlpha = SmoothStep(RequiredSupportDepth, RequiredSupportDepth + SupportFade, SupportDensity);

    if (SupportAlpha <= KINDA_SMALL_NUMBER)
    {
        return Density;
    }

    const float BackFade = FMath::Max(C.Voxel * 2.f, S.PlateauSurfaceEdgeFadeCm);
    const float BackAlpha = SmoothStep(-Depth, -Depth + BackFade, ActorX);
    const float SideAlpha = 1.f - SmoothStep(HalfW - BackFade, HalfW, FMath::Abs(ActorY));

    const float PlateauAlpha = FMath::Clamp(SupportAlpha * BackAlpha * SideAlpha, 0.f, 1.f);
    if (PlateauAlpha <= KINDA_SMALL_NUMBER)
    {
        return Density;
    }

    const float TopSurfaceZ = FMath::Max(CliffTopZ, ComputeTopPlateauSurfaceZ(LocalCm));
    const float EffectiveTopZ = FMath::Lerp(CliffTopZ, TopSurfaceZ, PlateauAlpha);
    const float UpperLayerDensity = FMath::Min(EffectiveTopZ - LocalCm.Z, LocalCm.Z - CliffTopZ);

    return FMath::Max(Density, UpperLayerDensity);
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
        density = FMath::Min(density, CliffTopZForBase - Local.Z);
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

            const float TopRoughnessScale = bInIntegratedTopDomain ? 0.15f : 1.f;

            const float SafeMask = FMath::Pow(surfMask, 1.0f + C.RoughMaskStrength * 2.0f);
            density += n * C.RoughAmp * SafeMask * TopRoughnessScale;
        }
    }

    return density;
}