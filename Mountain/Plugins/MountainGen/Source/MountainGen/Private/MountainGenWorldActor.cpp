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
#include <cfloat>

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

    Settings.Seed = Result.FinalSettings.Seed;

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();

    if (Result.MeshData.Vertices.Num() == 0 || Result.MeshData.Triangles.Num() == 0)
    {
        bAsyncWorking = false;

        if (bRegenQueued)
        {
            bRegenQueued = false;
            BuildChunkAndMesh();
        }
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

    // =========================
    // 공통: 난이도 프리셋 반영
    // =========================
    ApplyDifficultyPreset();

    const FMountainGenSettings SettingsSnapshot = Settings;

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
    const FVector WorldMax = SampleOriginWorld + FVector(
        SettingsSnapshot.ChunkX * Voxel,
        SettingsSnapshot.ChunkY * Voxel,
        SettingsSnapshot.ChunkZ * Voxel
    );

    const int32 InputSeed = SettingsSnapshot.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, SeedSearchTries);

    // =====================================================================
    // (A) 에디터 배치/Construction 경로
    // =====================================================================
#if WITH_EDITOR
    if (GetWorld())
    {
        const EWorldType::Type WT = GetWorld()->WorldType;
        const bool bIsEditorBuild = (WT == EWorldType::Editor || WT == EWorldType::EditorPreview);

        if (bIsEditorBuild)
        {
            ProcMesh->ClearAllMeshSections();
            ProcMesh->ClearCollisionConvexMeshes();

            // -------------------------
            // (1) FinalS 구성 + AutoTune
            // -------------------------
            FMountainGenSettings FinalS = SettingsSnapshot;
            ApplyDifficultyPresetTo(FinalS);

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
                    FRandomStream SeedRng((int32)((PTRINT)this) ^ 0x19D3A7F1);
                    const int32 StartSeed = SeedRng.RandRange(1, INT32_MAX);

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
                if (InputSeed <= 0)
                {
                    FRandomStream SeedRng((int32)((PTRINT)this) ^ 0x51A3B9D1);
                    FinalS.Seed = SeedRng.RandRange(1, INT32_MAX);
                }
                else
                {
                    FinalS.Seed = InputSeed;
                }
            }

            Settings.Seed = FinalS.Seed;

            // -------------------------
            // (2) Density 샘플링
            // -------------------------
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

            // -------------------------
            // (3) Marching Cubes
            // -------------------------
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

            if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
            {
                return;
            }

            // -------------------------
            // (4) Apply Mesh + Collision + Material
            // -------------------------
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
                FinalS.bCreateCollision
            );

            ProcMesh->SetCollisionEnabled(
                FinalS.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
            );

            if (VoxelMaterial)
                ProcMesh->SetMaterial(0, VoxelMaterial);

            return;
        }
    }
#endif // WITH_EDITOR

    // =====================================================================
    // (B) PIE/런타임 경로
    // =====================================================================
    if (bAsyncWorking)
    {
        bRegenQueued = true;
        return;
    }

    const int32 LocalBuildSerial = ++CurrentBuildSerial;
    bAsyncWorking = true;

    if (Settings.Seed <= 0)
    {
        FRandomStream SeedRng((int32)((PTRINT)this) ^ 0x7F4A1C2D ^ LocalBuildSerial);
        Settings.Seed = SeedRng.RandRange(1, INT32_MAX);
    }

    const int32 SeedSnapshotForAsync = Settings.Seed;

    TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);

    Async(EAsyncExecution::ThreadPool,
        [WeakThis, SettingsSnapshot, TerrainOriginWorld, WorldMin, WorldMax, ChunkOriginWorld, ActorWorld, SampleOriginWorld,
        SampleX, SampleY, SampleZ, Voxel, SeedSnapshotForAsync, TriesForSeedSearch, LocalBuildSerial]()
        {
            if (!WeakThis.IsValid()) return;

            // -------------------------
            // (1) FinalS 구성 + AutoTune
            // -------------------------
            FMountainGenSettings FinalS = SettingsSnapshot;
            AMountainGenWorldActor::ApplyDifficultyPresetTo(FinalS);

            const float FixedHeightAmp = FinalS.HeightAmpCm;
            const float FixedRadius = FinalS.EnvelopeRadiusCm;
            const float FixedBaseH = FinalS.BaseHeightCm;

            const int32 InputSeedRT = SeedSnapshotForAsync;

            if (FinalS.bAutoTune)
            {
                if (InputSeedRT > 0)
                {
                    FinalS.HeightAmpCm = FixedHeightAmp;
                    FinalS.EnvelopeRadiusCm = FixedRadius;
                    FinalS.BaseHeightCm = FixedBaseH;

                    MGDeriveParamsFromSeed(FinalS, InputSeedRT);
                    (void)MGFinalizeSettingsFromSeed(FinalS, TerrainOriginWorld, WorldMin, WorldMax);
                    FinalS.Seed = InputSeedRT;
                }
                else
                {
                    FRandomStream SeedRng((InputSeedRT ^ 0x19D3A7F1) + LocalBuildSerial);
                    const int32 StartSeed = SeedRng.RandRange(1, INT32_MAX);

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
                FinalS.Seed = (InputSeedRT <= 0) ? 1337 : InputSeedRT;
            }

            // -------------------------
            // (2) Density 샘플링
            // -------------------------
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

            // -------------------------
            // (3) Marching Cubes
            // -------------------------
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

            // -------------------------
            // (4) GameThread로 결과 전달
            // -------------------------
            AsyncTask(ENamedThreads::GameThread,
                [WeakThis, FinalS, MeshData = MoveTemp(MeshData), LocalBuildSerial]() mutable
                {
                    if (!WeakThis.IsValid()) return;
                    if (LocalBuildSerial != WeakThis->CurrentBuildSerial) return;

                    WeakThis->PendingResult.bValid = true;
                    WeakThis->PendingResult.BuildSerial = LocalBuildSerial;
                    WeakThis->PendingResult.FinalSettings = FinalS;
                    WeakThis->PendingResult.MeshData = MoveTemp(MeshData);
                });
        }
    );
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