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

    const FName PropName = PropertyChangedEvent.Property->GetFName();

    if (PropName == GET_MEMBER_NAME_CHECKED(FMountainGenSettings, Seed))
    {
        BuildChunkAndMesh();
    }
}
#endif