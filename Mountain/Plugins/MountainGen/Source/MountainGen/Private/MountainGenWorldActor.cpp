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
#include "Containers/Queue.h"

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

static FORCEINLINE bool MG_IsInside(int32 x, int32 y, int32 z, int32 SX, int32 SY, int32 SZ)
{
    return (x >= 0 && y >= 0 && z >= 0 && x < SX && y < SY && z < SZ);
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

    const float FrontX = FMath::Max(200.f, S.CliffThicknessCm);

    FVector TerrainOriginWorld = ActorWorld - FVector(FrontX, 0.f, 0.f);

    const float Band = FMath::Max(S.CliffDepthCm, Voxel * 2.f);
    const float HalfW = FMath::Max(1.f, S.CliffHalfWidthCm);
    const float H = FMath::Max(1.f, S.CliffHeightCm);

    const float XMinLocal = FrontX - Band;
    const float XMaxLocal = FrontX + Band;
    const float YMinLocal = -HalfW;
    const float YMaxLocal = +HalfW;
    const float ZMinLocal = S.BaseHeightCm;
    const float ZMaxLocal = S.BaseHeightCm + H;

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

        UI_Status(FString::Printf(TEXT("[MountainGen][Editor] Seed=%d"), Settings.Seed),
            2.0f, FColor::Green);

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

    BuildChunkAndMesh();
}
#endif