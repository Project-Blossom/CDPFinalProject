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

FString AMountainGenWorldActor::MakeMetricsLine(
    const FMountainGenSettings& S,
    const FMGMetrics& M,
    bool& bOutCaveOK,
    bool& bOutOverhangOK,
    bool& bOutSteepOK)
{
    bOutCaveOK = MG_InRange(M.CaveVoidRatio, S.Targets.CaveMin, S.Targets.CaveMax);
    bOutOverhangOK = MG_InRange(M.OverhangRatio, S.Targets.OverhangMin, S.Targets.OverhangMax);
    bOutSteepOK = MG_InRange(M.SteepRatio, S.Targets.SteepMin, S.Targets.SteepMax);

    return FString::Format(
        TEXT("Cave {0} [{1}~{2}] {3} | Overhang {4} [{5}~{6}] {7} | Steep {8} [{9}~{10}] {11}"),
        {
            M.CaveVoidRatio,  S.Targets.CaveMin,     S.Targets.CaveMax,     bOutCaveOK ? TEXT("OK") : TEXT("FAIL"),
            M.OverhangRatio,  S.Targets.OverhangMin, S.Targets.OverhangMax, bOutOverhangOK ? TEXT("OK") : TEXT("FAIL"),
            M.SteepRatio,     S.Targets.SteepMin,    S.Targets.SteepMax,    bOutSteepOK ? TEXT("OK") : TEXT("FAIL")
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
            bool bC = false, bO = false, bSt = false;
            const FString Line = MakeMetricsLine(S, EM, bC, bO, bSt);

            UI_Status(
                FString::Format(TEXT("[MountainGen][EditorMetrics] seed={0} | {1}"), { S.Seed, Line }),
                6.0f,
                (bC && bO && bSt) ? FColor::Green : FColor::Orange
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
                        bool bC = false, bO = false, bSt = false;
                        const FString Line = AMountainGenWorldActor::MakeMetricsLine(Cand, M, bC, bO, bSt);

                        AsyncTask(ENamedThreads::GameThread, [WeakThis, Attempt, CandSeed, Line, MaxSeedTries]()
                            {
                                if (!WeakThis.IsValid()) return;
                                WeakThis->UI_Status(
                                    FString::Format(TEXT("[MountainGen][SeedSearch] SATISFIED {0}/{1} seed={2} | {3}"),
                                        { Attempt, MaxSeedTries, CandSeed, Line }),
                                    6.0f,
                                    FColor::Green
                                );
                            });
                    }

                    FinalSeed = CandSeed;
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
                        bool bC = false, bO = false, bSt = false;
                        const FString Line = AMountainGenWorldActor::MakeMetricsLine(Cand, M, bC, bO, bSt);
                        const bool bOK = (bC && bO && bSt);

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