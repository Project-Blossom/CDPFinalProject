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
#include "MountainGenAutoTune.h"

#include "Async/Async.h"
#include "Math/RandomStream.h"

AMountainGenWorldActor::AMountainGenWorldActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetMobility(EComponentMobility::Movable);
    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = false;

    AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AMountainGenWorldActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyDifficultyPreset();
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

void AMountainGenWorldActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!PendingResult.bValid)
        return;

    FMGAsyncResult Result = MoveTemp(PendingResult);
    PendingResult.bValid = false;

    if (Result.BuildSerial != CurrentBuildSerial)
    {
        bAsyncWorking = false;
        return;
    }

    ApplyBuiltResult_GameThread(MoveTemp(Result));
}

void AMountainGenWorldActor::Regenerate()
{
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::SetSeed(int32 NewSeed)
{
    if (Settings.Seed == NewSeed) return;
    Settings.Seed = NewSeed;
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::RandomizeSeed()
{
    Settings.Seed = -1;
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::ApplyDifficultyPresetTo(FMountainGenSettings& S)
{
    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:
        S.Targets.CaveMin = 0.00f;   S.Targets.CaveMax = 0.04f;
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f;  S.Targets.SteepMax = 0.20f;
        break;

    case EMountainGenDifficulty::Normal:
        S.Targets.CaveMin = 0.01f;   S.Targets.CaveMax = 0.07f;
        S.Targets.OverhangMin = 0.02f; S.Targets.OverhangMax = 0.10f;
        S.Targets.SteepMin = 0.15f;  S.Targets.SteepMax = 0.35f;
        break;

    case EMountainGenDifficulty::Hard:
        S.Targets.CaveMin = 0.03f;   S.Targets.CaveMax = 0.12f;
        S.Targets.OverhangMin = 0.06f; S.Targets.OverhangMax = 0.18f;
        S.Targets.SteepMin = 0.25f;  S.Targets.SteepMax = 0.55f;
        break;

    case EMountainGenDifficulty::Extreme:
        S.Targets.CaveMin = 0.06f;   S.Targets.CaveMax = 0.20f;
        S.Targets.OverhangMin = 0.12f; S.Targets.OverhangMax = 0.30f;
        S.Targets.SteepMin = 0.40f;  S.Targets.SteepMax = 0.80f;
        break;

    default:
        S.Targets.CaveMin = 0.00f;   S.Targets.CaveMax = 0.04f;
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f;  S.Targets.SteepMax = 0.20f;
        break;
    }
}

void AMountainGenWorldActor::ApplyDifficultyPreset()
{
    ApplyDifficultyPresetTo(Settings);
}

void AMountainGenWorldActor::ApplyBuiltResult_GameThread(FMGAsyncResult&& Result)
{
    if (!ProcMesh)
    {
        bAsyncWorking = false;
        return;
    }

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();

    if (Result.MeshData.Vertices.Num() == 0 || Result.MeshData.Triangles.Num() == 0)
    {
        bAsyncWorking = false;
        return;
    }

    TArray<FLinearColor> Colors;
    Colors.SetNumZeroed(Result.MeshData.Vertices.Num());

    ProcMesh->CreateMeshSection_LinearColor(
        0,
        Result.MeshData.Vertices,
        Result.MeshData.Triangles,
        Result.MeshData.Normals,
        Result.MeshData.UV0,
        Colors,
        Result.MeshData.Tangents,
        Result.FinalSettings.bCreateCollision
    );

    ProcMesh->SetCollisionEnabled(
        Result.FinalSettings.bCreateCollision
        ? ECollisionEnabled::QueryAndPhysics
        : ECollisionEnabled::NoCollision
    );

    if (VoxelMaterial)
        ProcMesh->SetMaterial(0, VoxelMaterial);

    // 최종 seed만 반영
    Settings.Seed = Result.FinalSettings.Seed;

    bAsyncWorking = false;

    if (bRegenQueued)
    {
        bRegenQueued = false;
        BuildChunkAndMesh();
    }
}

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    if (bAsyncWorking)
    {
        bRegenQueued = true;
        return;
    }

    ApplyDifficultyPreset();

    const FMountainGenSettings SettingsSnapshot = Settings;
    const int32 LocalBuildSerial = ++CurrentBuildSerial;

    bAsyncWorking = true;

    // ------------------------------------------------------------
    // (0) 범위/샘플링 그리드
    // ------------------------------------------------------------
    const int32 SampleX = SettingsSnapshot.ChunkX + 1;
    const int32 SampleY = SettingsSnapshot.ChunkY + 1;
    const int32 SampleZ = SettingsSnapshot.ChunkZ + 1;

    const float Voxel = SettingsSnapshot.VoxelSizeCm;

    const float HalfX = (SettingsSnapshot.ChunkX * Voxel) * 0.5f;
    const float HalfY = (SettingsSnapshot.ChunkY * Voxel) * 0.5f;

    const FVector ActorWorld = GetActorLocation();
    const FVector TerrainOriginWorld = ActorWorld;

    const FVector SampleOriginWorld = ActorWorld + FVector(-HalfX, -HalfY, SettingsSnapshot.BaseHeightCm);
    const FVector ChunkOriginWorld = SampleOriginWorld;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = SampleOriginWorld + FVector(SettingsSnapshot.ChunkX * Voxel, SettingsSnapshot.ChunkY * Voxel, SettingsSnapshot.ChunkZ * Voxel);

    const int32 InputSeed = SettingsSnapshot.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, SeedSearchTries);

    TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);

    Async(EAsyncExecution::ThreadPool, [WeakThis, SettingsSnapshot, TerrainOriginWorld, WorldMin, WorldMax, ChunkOriginWorld, ActorWorld, SampleOriginWorld, SampleX, SampleY, SampleZ, Voxel, InputSeed, TriesForSeedSearch, LocalBuildSerial]()
        {
            if (!WeakThis.IsValid()) return;

            // ------------------------------------------------------------
            // (1) FinalS 구성 + 기존 AutoTune 유지
            // ------------------------------------------------------------
            FMountainGenSettings FinalS = SettingsSnapshot;
            AMountainGenWorldActor::ApplyDifficultyPresetTo(FinalS);

            const float FixedHeightAmp = FinalS.HeightAmpCm;
            const float FixedRadius = FinalS.EnvelopeRadiusCm;
            const float FixedBaseH = FinalS.BaseHeightCm;

            if (FinalS.bAutoTune)
            {
                if (InputSeed > 0)
                {
                    FinalS.HeightAmpCm = FixedHeightAmp;
                    FinalS.EnvelopeRadiusCm = FixedRadius;
                    FinalS.BaseHeightCm = FixedBaseH;

                    MGDeriveParamsFromSeed(FinalS, InputSeed);
                    (void)MGFinalizeSettingsFromSeed(FinalS, TerrainOriginWorld, WorldMin, WorldMax);
                    FinalS.Seed = InputSeed;
                }
                else
                {
                    const int32 StartSeed = FMath::RandRange(1, INT32_MAX);

                    FinalS.HeightAmpCm = FixedHeightAmp;
                    FinalS.EnvelopeRadiusCm = FixedRadius;
                    FinalS.BaseHeightCm = FixedBaseH;

                    const bool bFound = MGFindFinalSeedByFeedback(
                        FinalS,
                        TerrainOriginWorld,
                        WorldMin,
                        WorldMax,
                        StartSeed,
                        TriesForSeedSearch
                    );

                    if (!bFound)
                    {
                        MGDeriveParamsFromSeed(FinalS, StartSeed);
                        (void)MGFinalizeSettingsFromSeed(FinalS, TerrainOriginWorld, WorldMin, WorldMax);
                        FinalS.Seed = StartSeed;
                    }
                }
            }
            else
            {
                FinalS.Seed = (InputSeed <= 0) ? FMath::RandRange(1, INT32_MAX) : InputSeed;
            }

            // ------------------------------------------------------------
            // (2) FullGrid로 메시/콜리전 생성 전에 조건 판정 + Seed 재시도
            // ------------------------------------------------------------
            const int32 MaxAttempts = (FinalS.bRetrySeedUntilSatisfied)
                ? FMath::Max(1, FinalS.MaxSeedAttempts)
                : 1;

            auto ScoreToRange = [](float v, float mn, float mx)
                {
                    if (v < mn) return (mn - v);
                    if (v > mx) return (v - mx);
                    return 0.f;
                };

            float BestScore = FLT_MAX;
            FMountainGenSettings BestS = FinalS;

            FRandomStream RetryRng((FinalS.Seed > 0 ? FinalS.Seed : 1337) ^ 0xA13F2C9B);

            bool bSatisfied = false;

            for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
            {
                FMountainGenSettings Cand = FinalS;

                if (Attempt > 0)
                    Cand.Seed = RetryRng.RandRange(1, INT32_MAX);

                // 고정값 유지
                Cand.HeightAmpCm = FixedHeightAmp;
                Cand.EnvelopeRadiusCm = FixedRadius;
                Cand.BaseHeightCm = FixedBaseH;

                const FMGMetrics M = MGComputeMetricsFullGrid(Cand, TerrainOriginWorld, WorldMin, WorldMax);

                if (MGIsSatisfiedToTargets(Cand, M))
                {
                    FinalS = Cand;
                    bSatisfied = true;
                    break;
                }

                const float Score =
                    ScoreToRange(M.CaveVoidRatio, Cand.Targets.CaveMin, Cand.Targets.CaveMax) +
                    ScoreToRange(M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax) +
                    ScoreToRange(M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax);

                if (Score < BestScore)
                {
                    BestScore = Score;
                    BestS = Cand;
                }
            }

            if (!bSatisfied && FinalS.bRetrySeedUntilSatisfied)
                FinalS = BestS; // 만족 못 해도 가장 근접한 후보로 생성

            // ------------------------------------------------------------
            // (3) density 샘플링
            // ------------------------------------------------------------
            FVoxelChunk Chunk;
            Chunk.Init(SampleX, SampleY, SampleZ);

            FVoxelDensityGenerator Gen(FinalS, TerrainOriginWorld);

            for (int32 z = 0; z < SampleZ; ++z)
                for (int32 y = 0; y < SampleY; ++y)
                    for (int32 x = 0; x < SampleX; ++x)
                    {
                        const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                        Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                    }

            // ------------------------------------------------------------
            // (4) Marching Cubes
            // ------------------------------------------------------------
            FChunkMeshData MeshData;

            FVoxelMesher::BuildMarchingCubes(
                Chunk,
                FinalS.VoxelSizeCm,
                FinalS.IsoLevel,
                ChunkOriginWorld,
                ActorWorld,
                Gen,
                MeshData
            );

            // ------------------------------------------------------------
            // (5) 결과를 게임 스레드로
            // ------------------------------------------------------------
            AsyncTask(ENamedThreads::GameThread, [WeakThis, FinalS, MeshData = MoveTemp(MeshData), LocalBuildSerial]() mutable
                {
                    if (!WeakThis.IsValid()) return;
                    if (LocalBuildSerial != WeakThis->CurrentBuildSerial) return;

                    WeakThis->PendingResult.bValid = true;
                    WeakThis->PendingResult.BuildSerial = LocalBuildSerial;
                    WeakThis->PendingResult.FinalSettings = FinalS;
                    WeakThis->PendingResult.MeshData = MoveTemp(MeshData);
                });
        });
}

#if WITH_EDITOR
void AMountainGenWorldActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (!PropertyChangedEvent.Property) return;

    ApplyDifficultyPreset();
    BuildChunkAndMesh();
}
#endif