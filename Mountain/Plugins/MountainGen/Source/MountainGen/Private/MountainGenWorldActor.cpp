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

static constexpr int32 kAutoTuneFixedSeed = 1337;

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

    InvalidateAutoTuneCache();

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::RandomizeSeed()
{
    const int32 NewSeed = FMath::RandRange(0, INT32_MAX);
    SetSeed(NewSeed);
}

void AMountainGenWorldActor::InvalidateAutoTuneCache()
{
    bHasAutoTunedCache = false;
}

void AMountainGenWorldActor::ApplyDifficultyPresetTo(FMountainGenSettings& S)
{
    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:
        S.Targets.CaveMin = 0.00f;     S.Targets.CaveMax = 0.04f;
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f;    S.Targets.SteepMax = 0.20f;
        break;

    case EMountainGenDifficulty::Normal:
        S.Targets.CaveMin = 0.01f;     S.Targets.CaveMax = 0.07f;
        S.Targets.OverhangMin = 0.02f; S.Targets.OverhangMax = 0.10f;
        S.Targets.SteepMin = 0.15f;    S.Targets.SteepMax = 0.35f;
        break;

    case EMountainGenDifficulty::Hard:
        S.Targets.CaveMin = 0.03f;     S.Targets.CaveMax = 0.12f;
        S.Targets.OverhangMin = 0.06f; S.Targets.OverhangMax = 0.18f;
        S.Targets.SteepMin = 0.25f;    S.Targets.SteepMax = 0.55f;
        break;

    case EMountainGenDifficulty::Extreme:
        S.Targets.CaveMin = 0.06f;     S.Targets.CaveMax = 0.20f;
        S.Targets.OverhangMin = 0.12f; S.Targets.OverhangMax = 0.30f;
        S.Targets.SteepMin = 0.40f;    S.Targets.SteepMax = 0.80f;
        break;

    default:
        S.Targets.CaveMin = 0.00f;     S.Targets.CaveMax = 0.04f;
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f;    S.Targets.SteepMax = 0.20f;
        break;
    }
}

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();

    // ------------------------------------------------------------
    // (0) 샘플링 그리드 / 월드 범위
    // ------------------------------------------------------------
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
    const FVector SampleOriginWorld = ActorWorld + FVector(-HalfX, -HalfY, -HalfZ);
    const FVector TerrainOriginWorld = ActorWorld;
    const FVector ChunkOriginWorld = SampleOriginWorld;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = SampleOriginWorld + FVector(Settings.ChunkX * Voxel, Settings.ChunkY * Voxel, Settings.ChunkZ * Voxel);

    // ------------------------------------------------------------
    // (1) 입력(유저) 값 백업
    // ------------------------------------------------------------
    const int32 InputSeed = Settings.Seed;
    const EMountainGenDifficulty InputDiff = Settings.Difficulty;

    // ------------------------------------------------------------
    // (2) “생성에만 쓰는” 최종 세팅 FinalS 구성
    // ------------------------------------------------------------
    FMountainGenSettings FinalS = Settings;

    ApplyDifficultyPresetTo(FinalS);

    // ------------------------------------------------------------
    // (3) AutoTune (캐시)
    // ------------------------------------------------------------
    if (FinalS.bAutoTune)
    {
        if (!bHasAutoTunedCache)
        {
            FMountainGenSettings TuneS = FinalS;
            TuneS.Seed = kAutoTuneFixedSeed;

            MGTuneSettingsFeedback(TuneS, TerrainOriginWorld, WorldMin, WorldMax);

            CachedTunedSettings = TuneS;
            bHasAutoTunedCache = true;
        }

        const int32 KeepSeed = FinalS.Seed;
        FinalS = CachedTunedSettings;
        FinalS.Difficulty = InputDiff;
        ApplyDifficultyPresetTo(FinalS);
        FinalS.Seed = KeepSeed;
    }

    // ------------------------------------------------------------
    // (4) 최종 Seed 확정
    // ------------------------------------------------------------
    if (InputSeed < 0)
        FinalS.Seed = FMath::RandRange(0, INT32_MAX);
    else
        FinalS.Seed = InputSeed;

    Settings.Seed = FinalS.Seed;

    // ------------------------------------------------------------
    // (5) density 샘플링
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
    // (6) Marching Cubes
    // ------------------------------------------------------------
    FChunkMeshData MeshData;

    FVoxelMesher::BuildMarchingCubes(
        Chunk,
        FinalS.VoxelSizeCm,
        FinalS.IsoLevel,
        ChunkOriginWorld,
        Gen,
        MeshData
    );

    if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        return;

    // ------------------------------------------------------------
    // (7) Mesh apply
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

    InvalidateAutoTuneCache();
    BuildChunkAndMesh();
}
#endif