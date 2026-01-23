#include "MountainGenWorldActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"

#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerController.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"
#include "MountainGenMeshData.h"

// ==============================
// Feedback AutoTune
// ==============================

struct FMGRange
{
    float Min = 0.f;
    float Max = 0.f;

    bool Contains(float v) const { return v >= Min && v <= Max; }
    float Center() const { return 0.5f * (Min + Max); }
};

struct FMGTargets
{
    FMGRange CaveVoidRatio;  // 0..1
    FMGRange OverhangRatio;  // 0..1
    FMGRange SteepRatio;     // 0..1
};

struct FMGMetrics
{
    float CaveVoidRatio = 0.f;
    float OverhangRatio = 0.f;
    float SteepRatio = 0.f;

    int32 CaveSamples = 0;
    int32 SurfaceNearSamples = 0;
};

static FMGTargets MGTargetsFromDifficulty(EMountainGenDifficulty Diff)
{
    FMGTargets T;

    switch (Diff)
    {
    case EMountainGenDifficulty::Easy:
        T.CaveVoidRatio = { 0.00f, 0.04f };
        T.OverhangRatio = { 0.00f, 0.05f };
        T.SteepRatio = { 0.05f, 0.20f };
        break;

    case EMountainGenDifficulty::Normal:
        T.CaveVoidRatio = { 0.01f, 0.07f };
        T.OverhangRatio = { 0.02f, 0.10f };
        T.SteepRatio = { 0.15f, 0.35f };
        break;

    case EMountainGenDifficulty::Hard:
        T.CaveVoidRatio = { 0.03f, 0.12f };
        T.OverhangRatio = { 0.06f, 0.18f };
        T.SteepRatio = { 0.25f, 0.55f };
        break;

    case EMountainGenDifficulty::Extreme:
        T.CaveVoidRatio = { 0.06f, 0.20f };
        T.OverhangRatio = { 0.12f, 0.30f };
        T.SteepRatio = { 0.40f, 0.80f };
        break;

    default:
        T.CaveVoidRatio = { 0.00f, 0.04f };
        T.OverhangRatio = { 0.00f, 0.05f };
        T.SteepRatio = { 0.05f, 0.20f };
        break;
    }

    return T;
}

static FVector MGEstimateNormalFromDensity(const FVoxelDensityGenerator& Gen, const FVector& Pcm, float StepCm)
{
    const float dx = Gen.SampleDensity(Pcm + FVector(StepCm, 0, 0)) - Gen.SampleDensity(Pcm - FVector(StepCm, 0, 0));
    const float dy = Gen.SampleDensity(Pcm + FVector(0, StepCm, 0)) - Gen.SampleDensity(Pcm - FVector(0, StepCm, 0));
    const float dz = Gen.SampleDensity(Pcm + FVector(0, 0, StepCm)) - Gen.SampleDensity(Pcm - FVector(0, 0, StepCm));

    FVector N(dx, dy, dz);
    return N.GetSafeNormal();
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

    // ---- Cave void ratio ----
    const int32 CaveSamples = 8000;
    int32 Air = 0;

    float Z0, Z1;
    if (!S.bCaveHeightsAreAbsoluteWorldZ)
    {
        Z0 = WorldMinCm.Z + S.CaveMinHeightCm;
        Z1 = WorldMinCm.Z + S.CaveMaxHeightCm;
    }
    else
    {
        Z0 = S.CaveMinHeightCm;
        Z1 = S.CaveMaxHeightCm;
    }

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

    // ---- Overhang ratio + Steep ratio ----
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

static float MGClamp01(float x) { return FMath::Clamp(x, 0.f, 1.f); }

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
    const float BiasDelta = Err * GainBias;

    if (!S.bOverhangBiasIncreaseWhenValueIncreases)
        S.OverhangBias = MGClamp01(S.OverhangBias - BiasDelta);
    else
        S.OverhangBias = MGClamp01(S.OverhangBias + BiasDelta);

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

static bool MGTuneSettingsFeedback(
    FMountainGenSettings& InOutS,
    const FVector& TerrainOriginWorld,
    const FVector& WorldMinCm,
    const FVector& WorldMaxCm)
{
    const FMGTargets Targets = MGTargetsFromDifficulty(InOutS.Difficulty);
    const int32 MaxIters = FMath::Clamp(InOutS.AutoTuneMaxIters, 1, 20);

    for (int32 Iter = 0; Iter < MaxIters; ++Iter)
    {
        const FMGMetrics M = MGComputeMetricsQuick(InOutS, TerrainOriginWorld, WorldMinCm, WorldMaxCm);

        const bool bCaveOK = Targets.CaveVoidRatio.Contains(M.CaveVoidRatio);
        const bool bOverOK = Targets.OverhangRatio.Contains(M.OverhangRatio);
        const bool bSteepOK = Targets.SteepRatio.Contains(M.SteepRatio);

        if (bCaveOK && bOverOK && bSteepOK)
            return true;

        if (!bCaveOK)  MGFeedbackStep_Cave(InOutS, Targets.CaveVoidRatio, M.CaveVoidRatio);
        if (!bOverOK)  MGFeedbackStep_Overhang(InOutS, Targets.OverhangRatio, M.OverhangRatio);
        if (!bSteepOK) MGFeedbackStep_Steep(InOutS, Targets.SteepRatio, M.SteepRatio);

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

static constexpr int32 kAutoTuneFixedSeed = 1337;

static void MGCopyTunedParams_KeepSeed(FMountainGenSettings& Dst, const FMountainGenSettings& SrcTuned)
{
    const int32 KeepSeed = Dst.Seed;
    Dst = SrcTuned;
    Dst.Seed = KeepSeed;
}

// ==============================
// Actor impl
// ==============================

AMountainGenWorldActor::AMountainGenWorldActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetMobility(EComponentMobility::Movable);
    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = false;

    AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AMountainGenWorldActor::InvalidateAutoTuneCache()
{
    bHasAutoTunedCache = false;
}

void AMountainGenWorldActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 에디터에서 값이 바뀐 상태 반영 위해 무효화
    InvalidateAutoTuneCache();

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::BeginPlay()
{
    Super::BeginPlay();

    if (bEnableRandomSeedKey)
    {
        APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        if (PC)
        {
            EnableInput(PC);

            if (InputComponent)
            {
                InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AMountainGenWorldActor::RandomizeSeed);
                InputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &AMountainGenWorldActor::RandomizeSeed);
            }
        }
    }
}

void AMountainGenWorldActor::Regenerate()
{
    InvalidateAutoTuneCache();
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::SetSeed(int32 NewSeed)
{
    if (Settings.Seed == NewSeed) return;
    Settings.Seed = NewSeed;

    InvalidateAutoTuneCache();
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::RandomizeSeed()
{
    const int32 NewSeed = FMath::RandRange(0, INT32_MAX);
    SetSeed(NewSeed);
}

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();

    const int32 SampleX = Settings.ChunkX + 1;
    const int32 SampleY = Settings.ChunkY + 1;
    const int32 SampleZ = Settings.ChunkZ + 1;

    FVoxelChunk Chunk;
    Chunk.Init(SampleX, SampleY, SampleZ);

    const float Voxel = Settings.VoxelSizeCm;

    const float HalfX = (Settings.ChunkX * Voxel) * 0.5f;
    const float HalfY = (Settings.ChunkY * Voxel) * 0.5f;
    const float HalfZ = (Settings.ChunkZ * Voxel) * 0.5f;

    const FVector ActorWorld = GetActorLocation();
    const float BaseZ = Settings.BaseHeightCm;
    const FVector SampleOriginWorld = ActorWorld + FVector(-HalfX, -HalfY, BaseZ);

    const FVector TerrainOriginWorld = ActorWorld;

    // ============================================================
    // AutoTune은 한 번만 계산해서 CachedTunedSettings에 저장
    // ============================================================
    if (Settings.bAutoTune)
    {
        if (!bHasAutoTunedCache)
        {
            const FVector WorldMin = SampleOriginWorld;
            const FVector WorldMax =
                SampleOriginWorld + FVector(Settings.ChunkX * Voxel, Settings.ChunkY * Voxel, Settings.ChunkZ * Voxel);

            FMountainGenSettings TuneS = Settings;
            TuneS.Seed = kAutoTuneFixedSeed; // 튠 측정은 고정 시드

            MGTuneSettingsFeedback(TuneS, TerrainOriginWorld, WorldMin, WorldMax);

            CachedTunedSettings = TuneS;
            bHasAutoTunedCache = true;
        }

        // 튠 결과를 적용하되 Seed는 유지
        MGCopyTunedParams_KeepSeed(Settings, CachedTunedSettings);
    }

    // ============================================================
    // Seed < 0 이면 여기서 랜덤 확정
    // ============================================================
    if (Settings.Seed < 0)
    {
        Settings.Seed = FMath::RandRange(0, INT32_MAX);
    }

    // ============================================================
    // 최종 Settings로 밀도 샘플링
    // ============================================================
    FVoxelDensityGenerator Gen(Settings, TerrainOriginWorld);

    for (int32 z = 0; z < SampleZ; ++z)
        for (int32 y = 0; y < SampleY; ++y)
            for (int32 x = 0; x < SampleX; ++x)
            {
                const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
            }

    FChunkMeshData MeshData;
    const FVector ChunkOriginWorld = SampleOriginWorld;

    FVoxelMesher::BuildMarchingCubes(
        Chunk,
        Settings.VoxelSizeCm,
        Settings.IsoLevel,
        ChunkOriginWorld,
        MeshData
    );

    if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        return;

    TArray<FLinearColor> Colors;
    Colors.SetNumZeroed(MeshData.Vertices.Num());

    ProcMesh->CreateMeshSection_LinearColor(
        0,
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.Normals,
        MeshData.UV0,
        Colors,
        MeshData.Tangents,
        Settings.bCreateCollision
    );

    ProcMesh->SetCollisionEnabled(
        Settings.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
    );

    if (VoxelMaterial)
        ProcMesh->SetMaterial(0, VoxelMaterial);
}

#if WITH_EDITOR
void AMountainGenWorldActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (!PropertyChangedEvent.Property) return;

    InvalidateAutoTuneCache();
    BuildChunkAndMesh();
}
#endif