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
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "Async/ParallelFor.h"

struct FMGLockedPreset
{
    FMGTargets Targets;

    float BaseField3DStrengthCm = 0.f;
    float BaseField3DScaleCm = 0.f;
    int32 BaseField3DOctaves = 0;
    float DetailScaleCm = 0.f;
    int32 DetailOctaves = 0;

    float VolumeStrength = 0.f;
    float OverhangScaleCm = 0.f;
    float OverhangBias = 0.f;
    float OverhangDepthCm = 0.f;
    float OverhangFadeCm = 0.f;

    float GravityStrength = 0.f;
    float GravityScale = 0.f;

    float WarpPatchSizeCm = 0.f;
    float WarpPatchAmpCm = 0.f;
    float WarpStrength = 0.f;

    bool  bUseCliffBase = false;
    float CliffHalfWidthCm = 0.f;
    float CliffHeightCm = 0.f;
    float CliffThicknessCm = 0.f;
    float CliffDepthCm = 0.f;
    float FrontBandDepthCm = 0.f;
    float CliffSurfaceScaleCm = 0.f;
    float CliffSurfaceAmpCm = 0.f;

    bool  bEnableCaves = false;
    float CaveStrength = 0.f;
    float CaveScaleCm = 0.f;
    float CaveThreshold = 0.f;
    float CaveBand = 0.f;
    float CaveDepthCm = 0.f;
    float CaveNearSurfaceCm = 0.f;

    float CaveTileSizeCm = 0.f;
    float CaveDiameterCm = 0.f;
    int32 CaveMinSolidNeighbors = 0;

    int32 CavesPerTile_Easy = 0;
    int32 CavesPerTile_Normal = 0;
    int32 CavesPerTile_Hard = 0;
    int32 CavesPerTile_Extreme = 0;
};

static void SaveLockedPreset(const FMountainGenSettings& Src, FMGLockedPreset& Out)
{
    Out.Targets = Src.Targets;

    Out.BaseField3DStrengthCm = Src.BaseField3DStrengthCm;
    Out.BaseField3DScaleCm = Src.BaseField3DScaleCm;
    Out.BaseField3DOctaves = Src.BaseField3DOctaves;
    Out.DetailScaleCm = Src.DetailScaleCm;
    Out.DetailOctaves = Src.DetailOctaves;

    Out.VolumeStrength = Src.VolumeStrength;
    Out.OverhangScaleCm = Src.OverhangScaleCm;
    Out.OverhangBias = Src.OverhangBias;
    Out.OverhangDepthCm = Src.OverhangDepthCm;
    Out.OverhangFadeCm = Src.OverhangFadeCm;

    Out.GravityStrength = Src.GravityStrength;
    Out.GravityScale = Src.GravityScale;

    Out.WarpPatchSizeCm = Src.WarpPatchSizeCm;
    Out.WarpPatchAmpCm = Src.WarpPatchAmpCm;
    Out.WarpStrength = Src.WarpStrength;

    Out.bUseCliffBase = Src.bUseCliffBase;
    Out.CliffHalfWidthCm = Src.CliffHalfWidthCm;
    Out.CliffHeightCm = Src.CliffHeightCm;
    Out.CliffThicknessCm = Src.CliffThicknessCm;
    Out.CliffDepthCm = Src.CliffDepthCm;
    Out.FrontBandDepthCm = Src.FrontBandDepthCm;
    Out.CliffSurfaceScaleCm = Src.CliffSurfaceScaleCm;
    Out.CliffSurfaceAmpCm = Src.CliffSurfaceAmpCm;

    Out.bEnableCaves = Src.bEnableCaves;
    Out.CaveStrength = Src.CaveStrength;
    Out.CaveScaleCm = Src.CaveScaleCm;
    Out.CaveThreshold = Src.CaveThreshold;
    Out.CaveBand = Src.CaveBand;
    Out.CaveDepthCm = Src.CaveDepthCm;
    Out.CaveNearSurfaceCm = Src.CaveNearSurfaceCm;

    Out.CaveTileSizeCm = Src.CaveTileSizeCm;
    Out.CaveDiameterCm = Src.CaveDiameterCm;
    Out.CaveMinSolidNeighbors = Src.CaveMinSolidNeighbors;

    Out.CavesPerTile_Easy = Src.CavesPerTile_Easy;
    Out.CavesPerTile_Normal = Src.CavesPerTile_Normal;
    Out.CavesPerTile_Hard = Src.CavesPerTile_Hard;
    Out.CavesPerTile_Extreme = Src.CavesPerTile_Extreme;
}

static void RestoreLockedPreset(FMountainGenSettings& Dst, const FMGLockedPreset& L)
{
    Dst.Targets = L.Targets;

    Dst.BaseField3DStrengthCm = L.BaseField3DStrengthCm;
    Dst.BaseField3DScaleCm = L.BaseField3DScaleCm;
    Dst.BaseField3DOctaves = L.BaseField3DOctaves;
    Dst.DetailScaleCm = L.DetailScaleCm;
    Dst.DetailOctaves = L.DetailOctaves;

    Dst.VolumeStrength = L.VolumeStrength;
    Dst.OverhangScaleCm = L.OverhangScaleCm;
    Dst.OverhangBias = L.OverhangBias;
    Dst.OverhangDepthCm = L.OverhangDepthCm;
    Dst.OverhangFadeCm = L.OverhangFadeCm;

    Dst.GravityStrength = L.GravityStrength;
    Dst.GravityScale = L.GravityScale;

    Dst.WarpPatchSizeCm = L.WarpPatchSizeCm;
    Dst.WarpPatchAmpCm = L.WarpPatchAmpCm;
    Dst.WarpStrength = L.WarpStrength;

    Dst.bUseCliffBase = L.bUseCliffBase;
    Dst.CliffHalfWidthCm = L.CliffHalfWidthCm;
    Dst.CliffHeightCm = L.CliffHeightCm;
    Dst.CliffThicknessCm = L.CliffThicknessCm;
    Dst.CliffDepthCm = L.CliffDepthCm;
    Dst.FrontBandDepthCm = L.FrontBandDepthCm;
    Dst.CliffSurfaceScaleCm = L.CliffSurfaceScaleCm;
    Dst.CliffSurfaceAmpCm = L.CliffSurfaceAmpCm;

    Dst.bEnableCaves = L.bEnableCaves;
    Dst.CaveStrength = L.CaveStrength;
    Dst.CaveScaleCm = L.CaveScaleCm;
    Dst.CaveThreshold = L.CaveThreshold;
    Dst.CaveBand = L.CaveBand;
    Dst.CaveDepthCm = L.CaveDepthCm;
    Dst.CaveNearSurfaceCm = L.CaveNearSurfaceCm;

    Dst.CaveTileSizeCm = L.CaveTileSizeCm;
    Dst.CaveDiameterCm = L.CaveDiameterCm;
    Dst.CaveMinSolidNeighbors = L.CaveMinSolidNeighbors;

    Dst.CavesPerTile_Easy = L.CavesPerTile_Easy;
    Dst.CavesPerTile_Normal = L.CavesPerTile_Normal;
    Dst.CavesPerTile_Hard = L.CavesPerTile_Hard;
    Dst.CavesPerTile_Extreme = L.CavesPerTile_Extreme;
}

static void MG_CullMeshIslands(FChunkMeshData& Out, int32 MinTrisToKeep, bool bKeepLargestOnly)
{
    const int32 NumVerts = Out.Vertices.Num();
    const int32 NumTris = Out.Triangles.Num() / 3;
    if (NumVerts <= 0 || NumTris <= 0) return;

    TArray<TArray<int32>> TrisPerVert;
    TrisPerVert.SetNum(NumVerts);

    for (int32 t = 0; t < NumTris; ++t)
    {
        const int32 i0 = Out.Triangles[t * 3 + 0];
        const int32 i1 = Out.Triangles[t * 3 + 1];
        const int32 i2 = Out.Triangles[t * 3 + 2];

        if ((uint32)i0 >= (uint32)NumVerts || (uint32)i1 >= (uint32)NumVerts || (uint32)i2 >= (uint32)NumVerts)
            continue;

        TrisPerVert[i0].Add(t);
        TrisPerVert[i1].Add(t);
        TrisPerVert[i2].Add(t);
    }

    TArray<int32> TriComp;
    TriComp.Init(-1, NumTris);

    TArray<int32> CompTriCount;
    CompTriCount.Reserve(64);

    TQueue<int32> Q;
    int32 CompId = 0;

    for (int32 tStart = 0; tStart < NumTris; ++tStart)
    {
        if (TriComp[tStart] != -1) continue;

        int32 Count = 0;
        TriComp[tStart] = CompId;
        Q.Enqueue(tStart);

        while (!Q.IsEmpty())
        {
            int32 t;
            Q.Dequeue(t);
            ++Count;

            const int32 i0 = Out.Triangles[t * 3 + 0];
            const int32 i1 = Out.Triangles[t * 3 + 1];
            const int32 i2 = Out.Triangles[t * 3 + 2];

            auto PushNeighbors = [&](int32 v)
                {
                    for (int32 nt : TrisPerVert[v])
                    {
                        if (TriComp[nt] == -1)
                        {
                            TriComp[nt] = CompId;
                            Q.Enqueue(nt);
                        }
                    }
                };

            PushNeighbors(i0);
            PushNeighbors(i1);
            PushNeighbors(i2);
        }

        CompTriCount.Add(Count);
        ++CompId;
    }

    if (CompId <= 1) return;

    int32 LargestComp = 0;
    for (int32 c = 1; c < CompTriCount.Num(); ++c)
    {
        if (CompTriCount[c] > CompTriCount[LargestComp]) LargestComp = c;
    }

    TArray<uint8> KeepComp;
    KeepComp.Init(0, CompTriCount.Num());

    if (bKeepLargestOnly)
    {
        KeepComp[LargestComp] = 1;
    }
    else
    {
        for (int32 c = 0; c < CompTriCount.Num(); ++c)
        {
            if (CompTriCount[c] >= MinTrisToKeep) KeepComp[c] = 1;
        }
    }

    TArray<int32> NewTris;
    NewTris.Reserve(Out.Triangles.Num());

    TArray<int32> NewIndex;
    NewIndex.Init(-1, NumVerts);

    TArray<FVector> NewVerts;
    TArray<FVector> NewNormals;
    TArray<FVector2D> NewUV0;
    TArray<FProcMeshTangent> NewTangents;

    for (int32 t = 0; t < NumTris; ++t)
    {
        const int32 c = TriComp[t];
        if ((uint32)c >= (uint32)KeepComp.Num() || !KeepComp[c]) continue;

        const int32 old[3] = {
            Out.Triangles[t * 3 + 0],
            Out.Triangles[t * 3 + 1],
            Out.Triangles[t * 3 + 2]
        };

        int32 remap[3];
        for (int32 k = 0; k < 3; ++k)
        {
            const int32 ov = old[k];
            int32& ni = NewIndex[ov];
            if (ni == -1)
            {
                ni = NewVerts.Num();
                NewVerts.Add(Out.Vertices[ov]);
                NewNormals.Add(Out.Normals.IsValidIndex(ov) ? Out.Normals[ov] : FVector::UpVector);
                NewUV0.Add(Out.UV0.IsValidIndex(ov) ? Out.UV0[ov] : FVector2D::ZeroVector);
                NewTangents.Add(Out.Tangents.IsValidIndex(ov) ? Out.Tangents[ov] : FProcMeshTangent());
            }
            remap[k] = ni;
        }

        NewTris.Add(remap[0]);
        NewTris.Add(remap[1]);
        NewTris.Add(remap[2]);
    }

    Out.Vertices = MoveTemp(NewVerts);
    Out.Normals = MoveTemp(NewNormals);
    Out.UV0 = MoveTemp(NewUV0);
    Out.Tangents = MoveTemp(NewTangents);
    Out.Triangles = MoveTemp(NewTris);
}

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

void AMountainGenWorldActor::UI_Status(const FString& Msg, float Seconds, FColor Color) const
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, Seconds, Color, Msg);
    }
}

static bool MG_InRange(float V, float Min, float Max)
{
    return (V >= Min && V <= Max);
}

static int32 MG_CaveCountPerTile(const FMountainGenSettings& S)
{
    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:     return S.CavesPerTile_Easy;
    case EMountainGenDifficulty::Normal:   return S.CavesPerTile_Normal;
    case EMountainGenDifficulty::Hard:     return S.CavesPerTile_Hard;
    case EMountainGenDifficulty::Extreme:  return S.CavesPerTile_Extreme;
    default:                               return S.CavesPerTile_Easy;
    }
}

static FORCEINLINE bool MG_IsInside(int32 x, int32 y, int32 z, int32 SX, int32 SY, int32 SZ)
{
    return (x >= 0 && y >= 0 && z >= 0 && x < SX && y < SY && z < SZ);
}

static void MG_CarveCaves_PostProcess(
    FVoxelChunk& Chunk,
    const FMountainGenSettings& S,
    const FVector& SampleOriginWorld,
    float VoxelSizeCm)
{
    if (!S.bEnableCaves)
        return;

    const float Iso = S.IsoLevel;

    const float TileSizeCm = FMath::Max(1000.f, S.CaveTileSizeCm);
    const float CaveDiameter = FMath::Max(10.f, S.CaveDiameterCm);
    const float CaveRadiusCm = CaveDiameter * 0.5f;

    const int32 NPerTile = MG_CaveCountPerTile(S);
    if (NPerTile <= 0)
        return;

    const int32 SX = Chunk.SizeX;
    const int32 SY = Chunk.SizeY;
    const int32 SZ = Chunk.SizeZ;

    const float LocalSizeX = (SX - 1) * VoxelSizeCm;
    const float LocalSizeY = (SY - 1) * VoxelSizeCm;
    const float LocalSizeZ = (SZ - 1) * VoxelSizeCm;

    const int32 NumTilesX = FMath::Max(1, FMath::CeilToInt(LocalSizeX / TileSizeCm));
    const int32 NumTilesY = FMath::Max(1, FMath::CeilToInt(LocalSizeY / TileSizeCm));

    const int32 Rv = FMath::Max(1, FMath::CeilToInt(CaveRadiusCm / VoxelSizeCm));
    const int32 MinNeighbors = FMath::Clamp(S.CaveMinSolidNeighbors, 0, 6);

    const uint32 BaseSeed = (uint32)FMath::Max(1, S.Seed);
    const uint32 CaveBaseSeed = BaseSeed ^ 0xC0A51234u;

    auto WorldToVoxel = [&](const FVector& WorldPos) -> FIntVector
        {
            const FVector L = (WorldPos - SampleOriginWorld) / VoxelSizeCm;
            return FIntVector(FMath::RoundToInt(L.X), FMath::RoundToInt(L.Y), FMath::RoundToInt(L.Z));
        };

    auto VoxelToWorld = [&](int32 x, int32 y, int32 z) -> FVector
        {
            return SampleOriginWorld + FVector(x * VoxelSizeCm, y * VoxelSizeCm, z * VoxelSizeCm);
        };

    auto DensityAt = [&](int32 x, int32 y, int32 z) -> float
        {
            if (!MG_IsInside(x, y, z, SX, SY, SZ)) return Iso - 1.f;
            return Chunk.Get(x, y, z);
        };

    auto CountSolidNeighbors6 = [&](int32 x, int32 y, int32 z) -> int32
        {
            int32 c = 0;
            c += (DensityAt(x - 1, y, z) >= Iso) ? 1 : 0;
            c += (DensityAt(x + 1, y, z) >= Iso) ? 1 : 0;
            c += (DensityAt(x, y - 1, z) >= Iso) ? 1 : 0;
            c += (DensityAt(x, y + 1, z) >= Iso) ? 1 : 0;
            c += (DensityAt(x, y, z - 1) >= Iso) ? 1 : 0;
            c += (DensityAt(x, y, z + 1) >= Iso) ? 1 : 0;
            return c;
        };

    auto CarveSphereAtVoxel = [&](const FIntVector& C)
        {
            const FVector Cw = VoxelToWorld(C.X, C.Y, C.Z);

            for (int32 z = C.Z - Rv; z <= C.Z + Rv; ++z)
                for (int32 y = C.Y - Rv; y <= C.Y + Rv; ++y)
                    for (int32 x = C.X - Rv; x <= C.X + Rv; ++x)
                    {
                        if (!MG_IsInside(x, y, z, SX, SY, SZ)) continue;

                        const FVector P = VoxelToWorld(x, y, z);
                        if (FVector::Distance(P, Cw) > CaveRadiusCm) continue;

                        Chunk.Set(x, y, z, Iso - 1.f);
                    }
        };

    for (int32 Ty = 0; Ty < NumTilesY; ++Ty)
        for (int32 Tx = 0; Tx < NumTilesX; ++Tx)
        {
            const uint32 TileSeed =
                CaveBaseSeed ^
                (uint32)(Tx * 73856093) ^
                (uint32)(Ty * 19349663) ^
                0xA53C9E2Du;

            FRandomStream Rng((int32)TileSeed);

            for (int32 i = 0; i < NPerTile; ++i)
            {
                const int32 MaxTry = 24;

                for (int32 t = 0; t < MaxTry; ++t)
                {
                    const float TileOriginX = (float)Tx * TileSizeCm;
                    const float TileOriginY = (float)Ty * TileSizeCm;

                    const float X0 = TileOriginX;
                    const float Y0 = TileOriginY;
                    const float X1 = FMath::Min(TileOriginX + TileSizeCm, LocalSizeX);
                    const float Y1 = FMath::Min(TileOriginY + TileSizeCm, LocalSizeY);

                    if ((X1 - X0) < VoxelSizeCm * 2.f || (Y1 - Y0) < VoxelSizeCm * 2.f)
                        continue;

                    const float LocalX = Rng.FRandRange(X0 + 100.f, X1 - 100.f);
                    const float LocalY = Rng.FRandRange(Y0 + 100.f, Y1 - 100.f);
                    const float LocalZ = Rng.FRandRange(VoxelSizeCm * 2.f, LocalSizeZ - VoxelSizeCm * 2.f);

                    const FVector WorldPos = SampleOriginWorld + FVector(LocalX, LocalY, LocalZ);
                    const FIntVector C = WorldToVoxel(WorldPos);

                    if (!MG_IsInside(C.X, C.Y, C.Z, SX, SY, SZ)) continue;

                    if (DensityAt(C.X, C.Y, C.Z) < Iso) continue;
                    if (CountSolidNeighbors6(C.X, C.Y, C.Z) < MinNeighbors) continue;

                    CarveSphereAtVoxel(C);
                    break;
                }
            }
        }
}

FString AMountainGenWorldActor::MakeMetricsLine(
    const FMountainGenSettings& S,
    const FMGMetrics& M,
    bool& bOutOverhangOK,
    bool& bOutSteepOK)
{
    bOutOverhangOK = MG_InRange(M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax);
    bOutSteepOK = MG_InRange(M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax);

    return FString::Printf(
        TEXT("Overhang %.3f [%.3f~%.3f] %s | Steep %.3f [%.3f~%.3f] %s | Near=%d"),
        M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax, bOutOverhangOK ? TEXT("OK") : TEXT("FAIL"),
        M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax, bOutSteepOK ? TEXT("OK") : TEXT("FAIL"),
        M.SurfaceNearSamples
    );
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

    if (!bEnableRandomSeedKey) return;

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC) return;

    EnableInput(PC);

    if (!InputComponent)
    {
        InputComponent = NewObject<UInputComponent>(this, TEXT("MGInputComponent"));
        InputComponent->RegisterComponent();
        AddInstanceComponent(InputComponent);
        PC->PushInputComponent(InputComponent);
    }

    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AMountainGenWorldActor::RandomizeSeed);
    InputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &AMountainGenWorldActor::RandomizeSeed);

    UI_Status(TEXT("[MountainGen] 1 키: 시드 랜덤 변경"), 2.0f, FColor::Green);
}

void AMountainGenWorldActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!PendingResult.bValid)
        return;

    FMGAsyncResult Result = MoveTemp(PendingResult);
    PendingResult.bValid = false;

    if (Result.BuildSerial != InFlightBuildSerial)
        return;

    if (!ProcMesh)
    {
        bAsyncWorking = false;
        InFlightBuildSerial = 0;
        return;
    }

    ProcMesh->ClearAllMeshSections();
    ProcMesh->ClearCollisionConvexMeshes();

    if (Result.MeshData.Vertices.Num() == 0 || Result.MeshData.Triangles.Num() == 0)
    {
        UI_Status(TEXT("[MountainGen] 생성 실패: MeshData 비어있음"), 2.0f, FColor::Red);
        bAsyncWorking = false;
        InFlightBuildSerial = 0;
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

    Settings.Seed = Result.FinalSettings.Seed;

    UI_Status(FString::Printf(TEXT("[MountainGen] 시드 변경 완료: %d"), Settings.Seed), 2.5f, FColor::Yellow);

    bAsyncWorking = false;
    InFlightBuildSerial = 0;

    if (bRegenQueued)
    {
        bRegenQueued = false;
        BuildChunkAndMesh();
    }
}

void AMountainGenWorldActor::Regenerate()
{
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::SetSeed(int32 NewSeed)
{
    if (bAsyncWorking)
    {
        UI_Status(TEXT("[MountainGen] 작업 중... 완료 후 다시 시도"), 1.2f, FColor::Red);
        return;
    }

    if (Settings.Seed == NewSeed) return;

    Settings.Seed = NewSeed;
    UI_Status(FString::Printf(TEXT("[MountainGen] 시드 변경 요청: %d"), Settings.Seed), 1.5f, FColor::Cyan);

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::RandomizeSeed()
{
    if (bAsyncWorking)
    {
        UI_Status(TEXT("[MountainGen] 작업 중이라 시드 변경이 비활성화됨"), 1.2f, FColor::Orange);
        return;
    }

    UI_Status(TEXT("[MountainGen] 시드 변경(랜덤) 요청..."), 1.2f, FColor::Cyan);

    const uint64 T = FPlatformTime::Cycles64();
    const FVector L = GetActorLocation();

    uint32 Mix =
        (uint32)(T) ^
        (uint32)(T >> 32) ^
        (uint32)(PTRINT)this ^
        (uint32)FMath::RoundToInt(L.X) ^
        ((uint32)FMath::RoundToInt(L.Y) << 11) ^
        ((uint32)FMath::RoundToInt(L.Z) << 22) ^
        (uint32)(++CurrentBuildSerial * 977u);

    int32 NewSeed = (int32)(Mix & 0x7fffffff);
    if (NewSeed <= 0) NewSeed = 1;

    Settings.Seed = NewSeed;
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::ApplyDifficultyPresetTo(FMountainGenSettings& S)
{
    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f; S.Targets.SteepMax = 0.20f;

        S.BaseField3DStrengthCm = 5000.f;
        S.BaseField3DScaleCm = 22000.f;
        S.DetailScaleCm = 9000.f;
        S.VolumeStrength = 0.20f;
        S.OverhangScaleCm = 14000.f;
        S.OverhangFadeCm = 28000.f;
        S.OverhangBias = 0.70f;
        S.GravityStrength = 1.30f;
        S.GravityScale = 2.5f;
        break;

    case EMountainGenDifficulty::Normal:
        S.Targets.OverhangMin = 0.02f; S.Targets.OverhangMax = 0.10f;
        S.Targets.SteepMin = 0.15f; S.Targets.SteepMax = 0.35f;

        S.BaseField3DStrengthCm = 8500.f;
        S.BaseField3DScaleCm = 18000.f;
        S.DetailScaleCm = 7000.f;
        S.VolumeStrength = 0.55f;
        S.OverhangScaleCm = 10000.f;
        S.OverhangFadeCm = 20000.f;
        S.OverhangBias = 0.60f;
        S.GravityStrength = 1.10f;
        S.GravityScale = 2.2f;
        break;

    case EMountainGenDifficulty::Hard:
        S.Targets.OverhangMin = 0.06f; S.Targets.OverhangMax = 0.18f;
        S.Targets.SteepMin = 0.25f; S.Targets.SteepMax = 0.55f;

        S.BaseField3DStrengthCm = 12000.f;
        S.BaseField3DScaleCm = 16000.f;
        S.DetailScaleCm = 6000.f;
        S.VolumeStrength = 1.00f;
        S.OverhangScaleCm = 8000.f;
        S.OverhangFadeCm = 15000.f;
        S.OverhangBias = 0.55f;
        S.GravityStrength = 1.00f;
        S.GravityScale = 2.0f;
        break;

    case EMountainGenDifficulty::Extreme:
        S.Targets.OverhangMin = 0.12f; S.Targets.OverhangMax = 0.30f;
        S.Targets.SteepMin = 0.40f; S.Targets.SteepMax = 0.80f;

        S.BaseField3DStrengthCm = 17000.f;
        S.BaseField3DScaleCm = 12000.f;
        S.DetailScaleCm = 4200.f;
        S.VolumeStrength = 1.60f;
        S.OverhangScaleCm = 6000.f;
        S.OverhangFadeCm = 9000.f;
        S.OverhangBias = 0.48f;
        S.GravityStrength = 0.85f;
        S.GravityScale = 1.7f;
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

    UWorld* W = GetWorld();
    const bool bEditorLike = (!W || !W->IsGameWorld());

    // =========================================================
    // 0) Effective Settings
    // =========================================================
    FMountainGenSettings S = Settings;
    MGApplyDifficultyPreset(S);

    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);

    const FVector ActorWorld = GetActorLocation();
    const FVector TerrainOriginWorld = ActorWorld;

    float XMinLocal = -S.ChunkX * Voxel * 0.5f;
    float XMaxLocal = S.ChunkX * Voxel * 0.5f;
    float YMinLocal = -S.ChunkY * Voxel * 0.5f;
    float YMaxLocal = S.ChunkY * Voxel * 0.5f;
    float ZMinLocal = S.BaseHeightCm;
    float ZMaxLocal = S.BaseHeightCm + S.ChunkZ * Voxel;

    if (S.bUseCliffBase)
    {
        const float Band = FMath::Max(S.CliffDepthCm, Voxel * 2.f);
        const float HalfW = FMath::Max(1.f, S.CliffHalfWidthCm);
        const float H = FMath::Max(1.f, S.CliffHeightCm);
        const float FrontX = S.CliffThicknessCm;

        XMinLocal = FrontX - Band;
        XMaxLocal = FrontX + Band;

        YMinLocal = -HalfW;
        YMaxLocal = +HalfW;

        ZMinLocal = S.BaseHeightCm;
        ZMaxLocal = S.BaseHeightCm + H;
    }

    const FVector SampleOriginWorld = TerrainOriginWorld + FVector(XMinLocal, YMinLocal, ZMinLocal);
    const FVector ChunkOriginWorld = SampleOriginWorld;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = TerrainOriginWorld + FVector(XMaxLocal, YMaxLocal, ZMaxLocal);

    const int32 SampleX = FMath::Max(2, FMath::CeilToInt((XMaxLocal - XMinLocal) / Voxel) + 1);
    const int32 SampleY = FMath::Max(2, FMath::CeilToInt((YMaxLocal - YMinLocal) / Voxel) + 1);
    const int32 SampleZ = FMath::Max(2, FMath::CeilToInt((ZMaxLocal - ZMinLocal) / Voxel) + 1);

    // MetricsStep 기본값
    if (S.MetricsStepCm <= 0.f)
    {
        S.MetricsStepCm = FMath::Max(400.f, S.VoxelSizeCm * 2.f);
    }

    const int32 InputSeed = S.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, S.SeedSearchTries);

    UI_Status(
        FString::Printf(TEXT("[MountainGen] PATH=%s  SeedSearchTries=%d  Sample=%dx%dx%d"),
            bEditorLike ? TEXT("EditorLike") : TEXT("RuntimeAsync"),
            TriesForSeedSearch, SampleX, SampleY, SampleZ),
        4.0f,
        bEditorLike ? FColor::Yellow : FColor::Green
    );

    // =========================================================
    // (A) Editor: 동기 생성
    // =========================================================
    if (bEditorLike)
    {
        bAsyncWorking = false;
        bRegenQueued = false;
        InFlightBuildSerial = 0;

        // 1) AutoTune
        if (S.bAutoTune)
        {
            MGAutoTuneIntentParams(S, TerrainOriginWorld, WorldMin, WorldMax);
            MGClampToDifficultyBounds(S);
        }

        // 2) SeedSearch
        auto DebugPrint = [this](const FString& Msg, float Sec, FColor Col)
            {
                this->UI_Status(Msg, Sec, Col);
            };

        const int32 FinalSeed =
            MGSearchSeedForTargets(
                S,
                TerrainOriginWorld,
                WorldMin, WorldMax,
                InputSeed,
                TriesForSeedSearch,
                S.bRetrySeedUntilSatisfied,
                S.MaxSeedAttempts,
                bDebugSeedSearch,
                DebugPrintEveryNAttempt,
                DebugPrint
            );

        S.Seed = FinalSeed;
        MGDeriveReproducibleDomainFromSeed(S, FinalSeed);

        // 디버그 Metrics
        {
            const FMGMetrics M = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);

            const bool bO = (M.OverhangRatio >= S.Targets.OverhangMin && M.OverhangRatio <= S.Targets.OverhangMax);
            const bool bSt = (M.SteepRatio >= S.Targets.SteepMin && M.SteepRatio <= S.Targets.SteepMax);

            UI_Status(
                FString::Printf(TEXT("[MountainGen][EditorMetrics] seed=%d | Over %.3f [%.2f~%.2f] %s | Steep %.3f [%.2f~%.2f] %s | Near=%d"),
                    S.Seed,
                    M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax, bO ? TEXT("OK") : TEXT("FAIL"),
                    M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax, bSt ? TEXT("OK") : TEXT("FAIL"),
                    M.SurfaceNearSamples),
                6.0f,
                (bO && bSt) ? FColor::Green : FColor::Orange
            );
        }

        // 3) Density -> Meshing
        FVoxelChunk Chunk;
        Chunk.Init(SampleX, SampleY, SampleZ);

        FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

        ParallelFor(SampleZ, [&](int32 z)
            {
                for (int32 y = 0; y < SampleY; ++y)
                    for (int32 x = 0; x < SampleX; ++x)
                    {
                        const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                        Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                    }
            });

        FChunkMeshData MeshData;
        FVoxelMesher::BuildMarchingCubes(
            Chunk,
            S.VoxelSizeCm,
            S.IsoLevel,
            ChunkOriginWorld,
            ActorWorld,
            Gen,
            MeshData
        );

        MG_CullMeshIslands(MeshData, 200, true);

        ProcMesh->ClearAllMeshSections();
        ProcMesh->ClearCollisionConvexMeshes();

        if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        {
            UI_Status(TEXT("[MountainGen][Editor] MeshData 비어있음 (생성 실패/섬 제거로 모두 삭제됨)"), 2.0f, FColor::Red);
            return;
        }

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
            S.bCreateCollision
        );

        ProcMesh->SetCollisionEnabled(
            S.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
        );

        if (VoxelMaterial)
            ProcMesh->SetMaterial(0, VoxelMaterial);

        // Settings에는 최종 seed만 반영
        Settings.Seed = S.Seed;

        UI_Status(FString::Printf(TEXT("[MountainGen][Editor] Seed=%d (CliffBase=%s)"),
            Settings.Seed, S.bUseCliffBase ? TEXT("true") : TEXT("false")), 2.0f, FColor::Green);

        return;
    }

    // =========================================================
    // (B) Runtime: 비동기
    // =========================================================
    if (bAsyncWorking)
    {
        bRegenQueued = true;
        UI_Status(TEXT("[MountainGen] 작업 중 → 재생성 큐"), 1.5f, FColor::Orange);
        return;
    }

    const int32 LocalBuildSerial = ++CurrentBuildSerial;
    bAsyncWorking = true;
    InFlightBuildSerial = LocalBuildSerial;

    UI_Status(TEXT("[MountainGen] AutoTune/SeedSearch/생성 시작"), 2.0f, FColor::Cyan);

    TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);

    Async(EAsyncExecution::ThreadPool,
        [WeakThis,
        S, TerrainOriginWorld,
        WorldMin, WorldMax,
        ChunkOriginWorld, ActorWorld, SampleOriginWorld,
        SampleX, SampleY, SampleZ, Voxel,
        InputSeed, TriesForSeedSearch, LocalBuildSerial]() mutable
        {
            if (!WeakThis.IsValid()) return;

            // 1) AutoTune
            if (S.bAutoTune)
            {
                MGAutoTuneIntentParams(S, TerrainOriginWorld, WorldMin, WorldMax);
                MGClampToDifficultyBounds(S);
            }

            // 2) SeedSearch
            auto DebugPrint = [WeakThis](const FString& Msg, float Sec, FColor Col)
                {
                    AsyncTask(ENamedThreads::GameThread, [WeakThis, Msg, Sec, Col]()
                        {
                            if (!WeakThis.IsValid()) return;
                            WeakThis->UI_Status(Msg, Sec, Col);
                        });
                };

            const int32 FinalSeed =
                MGSearchSeedForTargets(
                    S,
                    TerrainOriginWorld,
                    WorldMin, WorldMax,
                    InputSeed,
                    TriesForSeedSearch,
                    S.bRetrySeedUntilSatisfied,
                    S.MaxSeedAttempts,
                    WeakThis->bDebugSeedSearch,
                    WeakThis->DebugPrintEveryNAttempt,
                    DebugPrint
                );

            S.Seed = FinalSeed;
            MGDeriveReproducibleDomainFromSeed(S, FinalSeed);

            // 3) Density -> Meshing
            FVoxelChunk Chunk;
            Chunk.Init(SampleX, SampleY, SampleZ);

            FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

            ParallelFor(SampleZ, [&](int32 z)
                {
                    for (int32 y = 0; y < SampleY; ++y)
                        for (int32 x = 0; x < SampleX; ++x)
                        {
                            const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                            Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                        }
                });

            FChunkMeshData MeshData;
            FVoxelMesher::BuildMarchingCubes(
                Chunk,
                S.VoxelSizeCm,
                S.IsoLevel,
                ChunkOriginWorld,
                ActorWorld,
                Gen,
                MeshData
            );

            MG_CullMeshIslands(MeshData, 200, true);

            // 4) 결과 전달
            AsyncTask(ENamedThreads::GameThread,
                [WeakThis, FinalS = S, MeshData = MoveTemp(MeshData), LocalBuildSerial]() mutable
                {
                    if (!WeakThis.IsValid()) return;
                    if (WeakThis->InFlightBuildSerial != LocalBuildSerial) return;

                    WeakThis->PendingResult.bValid = true;
                    WeakThis->PendingResult.BuildSerial = LocalBuildSerial;
                    WeakThis->PendingResult.FinalSettings = FinalS;
                    WeakThis->PendingResult.MeshData = MoveTemp(MeshData);

                    WeakThis->UI_Status(
                        FString::Printf(TEXT("[MountainGen] 시드 확정: %d"), FinalS.Seed),
                        2.5f,
                        FColor::Green
                    );
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