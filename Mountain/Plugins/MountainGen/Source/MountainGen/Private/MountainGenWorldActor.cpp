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
    const float CaveDiameterCm = FMath::Max(10.f, S.CaveDiameterCm);
    const float CaveRadiusCm = CaveDiameterCm * 0.5f;

    const int32 NPerTile = MG_CaveCountPerTile(S);
    if (NPerTile <= 0)
        return;

    const int32 SX = Chunk.SizeX;
    const int32 SY = Chunk.SizeY;
    const int32 SZ = Chunk.SizeZ;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = SampleOriginWorld + FVector(
        (SX - 1) * VoxelSizeCm,
        (SY - 1) * VoxelSizeCm,
        (SZ - 1) * VoxelSizeCm
    );

    const int32 TileMinX = FMath::FloorToInt(WorldMin.X / TileSizeCm);
    const int32 TileMaxX = FMath::FloorToInt(WorldMax.X / TileSizeCm);
    const int32 TileMinY = FMath::FloorToInt(WorldMin.Y / TileSizeCm);
    const int32 TileMaxY = FMath::FloorToInt(WorldMax.Y / TileSizeCm);

    const int32 Rv = FMath::Max(1, FMath::CeilToInt(CaveRadiusCm / VoxelSizeCm));
    const int32 MinNeighbors = FMath::Clamp(S.CaveMinSolidNeighbors, 0, 6);

    auto WorldToVoxel = [&](const FVector& W)
        {
            const FVector L = (W - SampleOriginWorld) / VoxelSizeCm;
            return FIntVector(FMath::RoundToInt(L.X), FMath::RoundToInt(L.Y), FMath::RoundToInt(L.Z));
        };

    auto VoxelToWorld = [&](int32 x, int32 y, int32 z)
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
                        const float Dist = FVector::Distance(P, Cw);
                        if (Dist > CaveRadiusCm) continue;

                        Chunk.Set(x, y, z, Iso - 1.f); // air
                    }
        };

    for (int32 Ty = TileMinY; Ty <= TileMaxY; ++Ty)
        for (int32 Tx = TileMinX; Tx <= TileMaxX; ++Tx)
        {
            const uint32 TileSeed =
                (uint32)FMath::Max(1, S.Seed) ^
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

                    const float X = Rng.FRandRange(TileOriginX + 100.f, TileOriginX + TileSizeCm - 100.f);
                    const float Y = Rng.FRandRange(TileOriginY + 100.f, TileOriginY + TileSizeCm - 100.f);
                    const float Z = Rng.FRandRange(WorldMin.Z + VoxelSizeCm * 2.f, WorldMax.Z - VoxelSizeCm * 2.f);

                    const FIntVector C = WorldToVoxel(FVector(X, Y, Z));
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

    return FString::Format(
        TEXT("Overhang {0} [{1}~{2}] {3} | Steep {4} [{5}~{6}] {7}"),
        {
            M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax, bOutOverhangOK ? TEXT("OK") : TEXT("FAIL"),
            M.SteepRatio,    S.Targets.SteepMin,    S.Targets.SteepMax,    bOutSteepOK ? TEXT("OK") : TEXT("FAIL")
        }
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
    {
        return;
    }

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

    UI_Status(FString::Format(TEXT("[MountainGen] 시드 변경 완료: {0}"), { Settings.Seed }), 2.5f, FColor::Yellow);

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
    UI_Status(FString::Format(TEXT("[MountainGen] 시드 변경 요청: {0}"), { Settings.Seed }), 1.5f, FColor::Cyan);

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

    Settings.Seed = -1;
    BuildChunkAndMesh();
}

void AMountainGenWorldActor::ApplyDifficultyPresetTo(FMountainGenSettings& S)
{
    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f; S.Targets.SteepMax = 0.20f;
        break;

    case EMountainGenDifficulty::Normal:
        S.Targets.OverhangMin = 0.02f; S.Targets.OverhangMax = 0.10f;
        S.Targets.SteepMin = 0.15f; S.Targets.SteepMax = 0.35f;
        break;

    case EMountainGenDifficulty::Hard:
        S.Targets.OverhangMin = 0.06f; S.Targets.OverhangMax = 0.18f;
        S.Targets.SteepMin = 0.25f; S.Targets.SteepMax = 0.55f;
        break;

    case EMountainGenDifficulty::Extreme:
        S.Targets.OverhangMin = 0.12f; S.Targets.OverhangMax = 0.30f;
        S.Targets.SteepMin = 0.40f; S.Targets.SteepMax = 0.80f;
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

    // ---------------------------------------
    // 공통: 난이도 프리셋 적용 + 스냅샷
    // ---------------------------------------
    ApplyDifficultyPreset();

    FMountainGenSettings S = Settings;
    ApplyDifficultyPresetTo(S);

    const float Voxel = S.VoxelSizeCm;

    const float HalfX = (S.ChunkX * Voxel) * 0.5f;
    const float HalfY = (S.ChunkY * Voxel) * 0.5f;

    const FVector ActorWorld = GetActorLocation();
    const FVector TerrainOriginWorld = ActorWorld;

    const FVector SampleOriginWorld = ActorWorld + FVector(-HalfX, -HalfY, S.BaseHeightCm);
    const FVector ChunkOriginWorld = SampleOriginWorld;

    const FVector WorldMin = SampleOriginWorld;
    const FVector WorldMax = SampleOriginWorld + FVector(S.ChunkX * Voxel, S.ChunkY * Voxel, S.ChunkZ * Voxel);

    const int32 SampleX = S.ChunkX + 1;
    const int32 SampleY = S.ChunkY + 1;
    const int32 SampleZ = S.ChunkZ + 1;

    const int32 InputSeed = S.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, S.SeedSearchTries);

    UI_Status(
        FString::Format(TEXT("[MountainGen] PATH={0}  SeedSearchTries={1}  DebugEveryN={2}  DebugOn={3}"),
            { bEditorLike ? TEXT("EditorLike") : TEXT("RuntimeAsync"), TriesForSeedSearch, DebugPrintEveryNAttempt, bDebugSeedSearch ? 1 : 0 }),
        4.0f,
        bEditorLike ? FColor::Yellow : FColor::Green
    );

    // ---------------------------------------
    // (A) 에디터: 동기 생성 + 즉시 적용
    // ---------------------------------------
    if (bEditorLike)
    {
        bAsyncWorking = false;
        bRegenQueued = false;
        InFlightBuildSerial = 0;

        // 1) Seed 확정
        if (S.Seed <= 0)
        {
            const int32 Hash = (int32)((PTRINT)this) ^ (int32)ActorWorld.X ^ ((int32)ActorWorld.Y << 1) ^ ((int32)ActorWorld.Z << 2);
            FRandomStream Rng(Hash ^ 0x51A3B9D1);
            S.Seed = Rng.RandRange(1, INT32_MAX);
        }

        // AutoTune이면 최종 파라미터 확정
        if (S.bAutoTune)
        {
            const float FixedHeightAmp = S.HeightAmpCm;
            const float FixedRadius = S.EnvelopeRadiusCm;
            const float FixedBaseH = S.BaseHeightCm;

            MGDeriveParamsFromSeed(S, S.Seed);
            (void)MGFinalizeSettingsFromSeed(S, TerrainOriginWorld, WorldMin, WorldMax);

            S.HeightAmpCm = FixedHeightAmp;
            S.EnvelopeRadiusCm = FixedRadius;
            S.BaseHeightCm = FixedBaseH;
        }

        Settings.Seed = S.Seed;

        {
            const FMGMetrics EM = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);

            bool bO = false, bSt = false;
            const FString Line = MakeMetricsLine(S, EM, bO, bSt);

            UI_Status(
                FString::Format(TEXT("[MountainGen][EditorMetrics] seed={0} | {1}"), { S.Seed, Line }),
                6.0f,
                (bO && bSt) ? FColor::Green : FColor::Orange
            );
        }

        // 2) 샘플링
        FVoxelChunk Chunk;
        Chunk.Init(SampleX, SampleY, SampleZ);

        FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

        for (int32 z = 0; z < SampleZ; ++z)
            for (int32 y = 0; y < SampleY; ++y)
                for (int32 x = 0; x < SampleX; ++x)
                {
                    const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                    Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                }

        MG_CarveCaves_PostProcess(Chunk, S, SampleOriginWorld, Voxel);

        // 3) 메시 생성 + 즉시 적용
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

        ProcMesh->ClearAllMeshSections();
        ProcMesh->ClearCollisionConvexMeshes();

        if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        {
            UI_Status(TEXT("[MountainGen][Editor] MeshData 비어있음 (생성 실패)"), 2.0f, FColor::Red);
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

        UI_Status(FString::Format(TEXT("[MountainGen][Editor] Seed={0} (즉시 생성)"), { Settings.Seed }), 2.0f, FColor::Green);
        return;
    }

    // ---------------------------------------
    // (B) 런타임: 비동기
    // ---------------------------------------
    if (bAsyncWorking)
    {
        bRegenQueued = true;
        UI_Status(TEXT("[MountainGen] 작업 중 → 재생성 큐"), 1.5f, FColor::Orange);
        return;
    }

    const int32 LocalBuildSerial = ++CurrentBuildSerial;
    bAsyncWorking = true;
    InFlightBuildSerial = LocalBuildSerial;

    UI_Status(TEXT("[MountainGen] 시드 탐색/생성 시작"), 2.0f, FColor::Cyan);

    TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);

    Async(EAsyncExecution::ThreadPool,
        [WeakThis,
        S, TerrainOriginWorld, WorldMin, WorldMax,
        ChunkOriginWorld, ActorWorld, SampleOriginWorld,
        SampleX, SampleY, SampleZ, Voxel,
        InputSeed, TriesForSeedSearch, LocalBuildSerial]() mutable
        {
            if (!WeakThis.IsValid()) return;

            FMountainGenSettings BaseS = S;

            FRandomStream SeedRng;
            if (InputSeed > 0) SeedRng.Initialize(InputSeed ^ 0x1F3A9B2D);
            else               SeedRng.Initialize((int32)((PTRINT)WeakThis.Get()) ^ 0x19D3A7F1);

            int32 FinalSeed = (InputSeed > 0) ? InputSeed : SeedRng.RandRange(1, INT32_MAX);

            auto ScoreToRange = [](float v, float mn, float mx)
                {
                    if (v < mn) return (mn - v);
                    if (v > mx) return (v - mx);
                    return 0.f;
                };

            int32 BestSeed = FinalSeed;
            float BestScore = FLT_MAX;
            bool bSatisfied = false;

            const int32 MaxSeedTries = TriesForSeedSearch;

            for (int32 Attempt = 1; Attempt <= MaxSeedTries; ++Attempt)
            {
                const int32 CandSeed = (Attempt == 1 && InputSeed > 0)
                    ? InputSeed
                    : SeedRng.RandRange(1, INT32_MAX);

                FMountainGenSettings Cand = BaseS;
                Cand.Seed = CandSeed;

                const FMGMetrics M = MGComputeMetricsQuick(Cand, TerrainOriginWorld, WorldMin, WorldMax);

                if (MGIsSatisfiedToTargets(Cand, M))
                {
                    if (WeakThis->bDebugSeedSearch)
                    {
                        bool bO = false, bSt = false;
                        const FString Line = MakeMetricsLine(Cand, M, bO, bSt);

                        AsyncTask(ENamedThreads::GameThread, [WeakThis, Attempt, CandSeed, Line, MaxSeedTries, bO, bSt]()
                            {
                                if (!WeakThis.IsValid()) return;

                                const FColor Color = (bO && bSt) ? FColor::Green : FColor::Orange;

                                WeakThis->UI_Status(
                                    FString::Format(TEXT("[MountainGen][SeedSearch] SATISFIED {0}/{1} seed={2} | {3}"),
                                        { Attempt, MaxSeedTries, CandSeed, Line }),
                                    6.0f,
                                    Color
                                );
                            });
                    }

                    FinalSeed = CandSeed;
                    bSatisfied = true;
                    break;
                }

                const float Score =
                    ScoreToRange(M.OverhangRatio, Cand.Targets.OverhangMin, Cand.Targets.OverhangMax) +
                    ScoreToRange(M.SteepRatio, Cand.Targets.SteepMin, Cand.Targets.SteepMax);

                if (Score < BestScore)
                {
                    BestScore = Score;
                    BestSeed = CandSeed;
                }

                if (WeakThis->bDebugSeedSearch)
                {
                    const int32 EveryN = FMath::Max(1, WeakThis->DebugPrintEveryNAttempt);

                    const bool bShouldPrint =
                        (Attempt == 1) ||
                        (Attempt == MaxSeedTries) ||
                        ((Attempt % EveryN) == 0);

                    if (bShouldPrint)
                    {
                        bool bO = false, bSt = false;
                        const FString Line = MakeMetricsLine(Cand, M, bO, bSt);
                        const bool bOK = (bO && bSt);

                        AsyncTask(ENamedThreads::GameThread, [WeakThis, Attempt, CandSeed, Line, bOK, MaxSeedTries]()
                            {
                                if (!WeakThis.IsValid()) return;
                                const FColor Color = bOK ? FColor::Green : FColor::Orange;

                                WeakThis->UI_Status(
                                    FString::Format(TEXT("[MountainGen][SeedSearch] Try {0}/{1} seed={2} | {3}"),
                                        { Attempt, MaxSeedTries, CandSeed, Line }),
                                    6.0f,
                                    Color
                                );
                            });
                    }
                }
            }

            if (!bSatisfied)
            {
                FinalSeed = BestSeed;
                AsyncTask(ENamedThreads::GameThread, [WeakThis, FinalSeed]()
                    {
                        if (!WeakThis.IsValid()) return;
                        WeakThis->UI_Status(
                            FString::Format(TEXT("[MountainGen] 만족 실패 → 근접 후보 확정 (seed={0})"), { FinalSeed }),
                            6.0f, FColor::Red
                        );
                    });
            }

            // (2) 최종 설정 확정 + AutoTune
            FMountainGenSettings FinalS = BaseS;
            FinalS.Seed = FinalSeed;

            if (FinalS.bAutoTune)
            {
                const float FixedHeightAmp = FinalS.HeightAmpCm;
                const float FixedRadius = FinalS.EnvelopeRadiusCm;
                const float FixedBaseH = FinalS.BaseHeightCm;

                MGDeriveParamsFromSeed(FinalS, FinalSeed);
                (void)MGFinalizeSettingsFromSeed(FinalS, TerrainOriginWorld, WorldMin, WorldMax);

                FinalS.HeightAmpCm = FixedHeightAmp;
                FinalS.EnvelopeRadiusCm = FixedRadius;
                FinalS.BaseHeightCm = FixedBaseH;
            }

            // (3) Mesh Build
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

            MG_CarveCaves_PostProcess(Chunk, FinalS, SampleOriginWorld, Voxel);

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

            // (4) 결과 전달
            AsyncTask(ENamedThreads::GameThread,
                [WeakThis, FinalS, MeshData = MoveTemp(MeshData), LocalBuildSerial]() mutable
                {
                    if (!WeakThis.IsValid()) return;

                    if (WeakThis->InFlightBuildSerial != LocalBuildSerial)
                        return;

                    WeakThis->PendingResult.bValid = true;
                    WeakThis->PendingResult.BuildSerial = LocalBuildSerial;
                    WeakThis->PendingResult.FinalSettings = FinalS;
                    WeakThis->PendingResult.MeshData = MoveTemp(MeshData);

                    WeakThis->UI_Status(
                        FString::Format(TEXT("[MountainGen] 시드 확정: {0}"), { FinalS.Seed }),
                        3.0f, FColor::Green
                    );
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