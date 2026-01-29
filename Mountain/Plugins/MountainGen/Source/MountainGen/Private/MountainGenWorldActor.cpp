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

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();

    // ------------------------------------------------------------
    // (0) 범위/샘플링 그리드
    // ------------------------------------------------------------
    ApplyDifficultyPreset();

    const int32 SampleX = Settings.ChunkX + 1;
    const int32 SampleY = Settings.ChunkY + 1;
    const int32 SampleZ = Settings.ChunkZ + 1;

    FVoxelChunk Chunk;
    Chunk.Init(SampleX, SampleY, SampleZ);

    const float Voxel = Settings.VoxelSizeCm;

    const float HalfX = (Settings.ChunkX * Voxel) * 0.5f;
    const float HalfY = (Settings.ChunkY * Voxel) * 0.5f;

    const FVector ActorWorld = GetActorLocation();
    const FVector TerrainOriginWorld = ActorWorld;

    const FVector SampleOriginWorld = ActorWorld + FVector(-HalfX, -HalfY, Settings.BaseHeightCm);
    const FVector ChunkOriginWorld = SampleOriginWorld;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = SampleOriginWorld + FVector(Settings.ChunkX * Voxel, Settings.ChunkY * Voxel, Settings.ChunkZ * Voxel);

    // ------------------------------------------------------------
    // (1) 입력 Seed 보관
    // ------------------------------------------------------------
    const int32 InputSeed = Settings.Seed;

    // ------------------------------------------------------------
    // (2) FinalS 구성
    // ------------------------------------------------------------
    FMountainGenSettings FinalS = Settings;
    ApplyDifficultyPresetTo(FinalS);

    // ✅ 고정 조건(너 요구)
    const float FixedHeightAmp = FinalS.HeightAmpCm;         // 최대높이
    const float FixedRadius = FinalS.EnvelopeRadiusCm;    // 범위
    const float FixedBaseH = FinalS.BaseHeightCm;

    // ------------------------------------------------------------
    // (3) AutoTune
    // ------------------------------------------------------------
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
            const int32 Tries = FMath::Max(1, SeedSearchTries);

            FinalS.HeightAmpCm = FixedHeightAmp;
            FinalS.EnvelopeRadiusCm = FixedRadius;
            FinalS.BaseHeightCm = FixedBaseH;

            const bool bFound = MGFindFinalSeedByFeedback(
                FinalS,
                TerrainOriginWorld,
                WorldMin,
                WorldMax,
                StartSeed,
                Tries
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
            FinalS.Seed = FMath::RandRange(1, INT32_MAX);
        else
            FinalS.Seed = InputSeed;
    }

    Settings.Seed = FinalS.Seed;

    // ------------------------------------------------------------
    // (4) density 샘플링
    // ------------------------------------------------------------
    FVoxelDensityGenerator Gen(FinalS, TerrainOriginWorld);

    for (int32 z = 0; z < SampleZ; ++z)
        for (int32 y = 0; y < SampleY; ++y)
            for (int32 x = 0; x < SampleX; ++x)
            {
                const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
            }

    // ------------------------------------------------------------
    // (5) Marching Cubes
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

    if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        return;

    // ------------------------------------------------------------
    // (6) Mesh apply
    // ------------------------------------------------------------
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