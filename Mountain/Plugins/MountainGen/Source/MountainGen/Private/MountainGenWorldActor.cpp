#include "MountainGenWorldActor.h"

#include "ProceduralMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"

#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "Math/RandomStream.h"

#include "VoxelChunk.h"
#include "VoxelDensityGenerator.h"
#include "VoxelMesher.h"
#include "MountainGenAutoTune.h"

// ============================================================
// Weld
// ============================================================

struct FMGWeldKey
{
    int64 X, Y, Z;

    bool operator==(const FMGWeldKey& O) const
    {
        return X == O.X && Y == O.Y && Z == O.Z;
    }
};

FORCEINLINE uint32 GetTypeHash(const FMGWeldKey& K)
{
    uint32 H = ::GetTypeHash(K.X);
    H = HashCombine(H, ::GetTypeHash(K.Y));
    H = HashCombine(H, ::GetTypeHash(K.Z));
    return H;
}

static void MG_WeldVertices_Quantized(FChunkMeshData& M, float EpsilonCm)
{
    const int32 V = M.Vertices.Num();
    const int32 I = M.Triangles.Num();
    if (V <= 0 || I <= 0) return;

    EpsilonCm = FMath::Max(0.01f, EpsilonCm);

    TArray<int32> Rep;
    Rep.Init(-1, V);

    TArray<FVector> NewVerts;
    TArray<FVector> NewNormals;
    TArray<FVector2D> NewUV0;
    TArray<FProcMeshTangent> NewTangents;

    NewVerts.Reserve(V);
    NewNormals.Reserve(V);
    NewUV0.Reserve(V);
    NewTangents.Reserve(V);

    TMap<FMGWeldKey, int32> CellToRep;
    CellToRep.Reserve(V);

    auto QuantKey = [&](const FVector& P) -> FMGWeldKey
        {
            const double Inv = 1.0 / (double)EpsilonCm;
            return FMGWeldKey{
                FMath::RoundToInt64((double)P.X * Inv),
                FMath::RoundToInt64((double)P.Y * Inv),
                FMath::RoundToInt64((double)P.Z * Inv)
            };
        };

    for (int32 i = 0; i < V; ++i)
    {
        const FVector P = M.Vertices[i];
        const FMGWeldKey K = QuantKey(P);

        int32* Found = CellToRep.Find(K);
        if (!Found)
        {
            const int32 NewIdx = NewVerts.Num();
            CellToRep.Add(K, NewIdx);

            Rep[i] = NewIdx;
            NewVerts.Add(P);

            const FVector N = M.Normals.IsValidIndex(i) ? M.Normals[i] : FVector::UpVector;
            NewNormals.Add(N);

            const FVector2D UV = M.UV0.IsValidIndex(i) ? M.UV0[i] : FVector2D::ZeroVector;
            NewUV0.Add(UV);

            const FProcMeshTangent Tng = M.Tangents.IsValidIndex(i) ? M.Tangents[i] : FProcMeshTangent();
            NewTangents.Add(Tng);
        }
        else
        {
            const int32 RepIdx = *Found;
            Rep[i] = RepIdx;

            const FVector N = M.Normals.IsValidIndex(i) ? M.Normals[i] : FVector::UpVector;
            NewNormals[RepIdx] += N;
        }
    }

    for (int32 r = 0; r < NewNormals.Num(); ++r)
    {
        if (!NewNormals[r].Normalize())
            NewNormals[r] = FVector::UpVector;
    }

    TArray<int32> NewTris;
    NewTris.Reserve(I);

    const int32 NumTris = I / 3;
    for (int32 t = 0; t < NumTris; ++t)
    {
        const int32 a0 = M.Triangles[t * 3 + 0];
        const int32 b0 = M.Triangles[t * 3 + 1];
        const int32 c0 = M.Triangles[t * 3 + 2];

        if ((uint32)a0 >= (uint32)V || (uint32)b0 >= (uint32)V || (uint32)c0 >= (uint32)V)
            continue;

        const int32 a = Rep[a0];
        const int32 b = Rep[b0];
        const int32 c = Rep[c0];

        if (a == b || b == c || c == a)
            continue;

        NewTris.Add(a);
        NewTris.Add(b);
        NewTris.Add(c);
    }

    const int32 NewV = NewVerts.Num();

    TArray<int32> Used;
    Used.Init(0, NewV);
    for (int32 idx : NewTris)
        if ((uint32)idx < (uint32)NewV) Used[idx] = 1;

    TArray<int32> FinalRemap;
    FinalRemap.Init(-1, NewV);

    FChunkMeshData Out;
    Out.Vertices.Reserve(NewV);
    Out.Normals.Reserve(NewV);
    Out.UV0.Reserve(NewV);
    Out.Tangents.Reserve(NewV);

    for (int32 r = 0; r < NewV; ++r)
    {
        if (!Used[r]) continue;
        FinalRemap[r] = Out.Vertices.Num();
        Out.Vertices.Add(NewVerts[r]);
        Out.Normals.Add(NewNormals[r]);
        Out.UV0.Add(NewUV0[r]);
        Out.Tangents.Add(NewTangents[r]);
    }

    Out.Triangles.Reserve(NewTris.Num());
    for (int32 idx : NewTris)
    {
        const int32 ni = FinalRemap[idx];
        if (ni >= 0) Out.Triangles.Add(ni);
    }

    M = MoveTemp(Out);
}

// ============================================================
// Cull islands
// ============================================================

struct FDSU
{
    TArray<int32> Parent;
    TArray<uint8> Rank;

    explicit FDSU(int32 N)
    {
        Parent.SetNumUninitialized(N);
        Rank.Init(0, N);
        for (int32 i = 0; i < N; ++i) Parent[i] = i;
    }

    int32 Find(int32 x)
    {
        while (Parent[x] != x)
        {
            Parent[x] = Parent[Parent[x]];
            x = Parent[x];
        }
        return x;
    }

    void Union(int32 a, int32 b)
    {
        a = Find(a);
        b = Find(b);
        if (a == b) return;
        if (Rank[a] < Rank[b]) Swap(a, b);
        Parent[b] = a;
        if (Rank[a] == Rank[b]) Rank[a]++;
    }
};

static void MG_CullMeshIslands(FChunkMeshData& Out, int32 MinTrisToKeep, bool bKeepLargestOnly)
{
    const int32 V = Out.Vertices.Num();
    const int32 T = Out.Triangles.Num() / 3;
    if (V == 0 || T == 0) return;

    FDSU Dsu(V);
    auto Valid = [&](int32 i) { return (uint32)i < (uint32)V; };

    for (int32 t = 0; t < T; ++t)
    {
        int32 a = Out.Triangles[t * 3 + 0];
        int32 b = Out.Triangles[t * 3 + 1];
        int32 c = Out.Triangles[t * 3 + 2];
        if (!Valid(a) || !Valid(b) || !Valid(c)) continue;
        Dsu.Union(a, b);
        Dsu.Union(b, c);
        Dsu.Union(c, a);
    }

    TMap<int32, int32> RootCount;
    RootCount.Reserve(T);

    for (int32 t = 0; t < T; ++t)
    {
        int32 a = Out.Triangles[t * 3];
        if (!Valid(a)) continue;
        RootCount.FindOrAdd(Dsu.Find(a))++;
    }

    int32 KeepRoot = -1;
    if (bKeepLargestOnly)
    {
        int32 Best = 0;
        for (auto& K : RootCount)
            if (K.Value > Best) { Best = K.Value; KeepRoot = K.Key; }
    }

    auto Keep = [&](int32 r)
        {
            if (bKeepLargestOnly) return r == KeepRoot;
            const int32* C = RootCount.Find(r);
            return C && *C >= MinTrisToKeep;
        };

    TArray<int32> Remap; Remap.Init(-1, V);
    FChunkMeshData New;

    for (int32 t = 0; t < T; ++t)
    {
        int32 o[3] = {
            Out.Triangles[t * 3 + 0],
            Out.Triangles[t * 3 + 1],
            Out.Triangles[t * 3 + 2]
        };
        if (!Valid(o[0]) || !Valid(o[1]) || !Valid(o[2])) continue;
        if (!Keep(Dsu.Find(o[0]))) continue;

        int32 r[3];
        for (int i = 0; i < 3; i++)
        {
            int32& idx = Remap[o[i]];
            if (idx == -1)
            {
                idx = New.Vertices.Num();
                New.Vertices.Add(Out.Vertices[o[i]]);
                New.Normals.Add(Out.Normals.IsValidIndex(o[i]) ? Out.Normals[o[i]] : FVector::UpVector);
                New.UV0.Add(Out.UV0.IsValidIndex(o[i]) ? Out.UV0[o[i]] : FVector2D::ZeroVector);
                New.Tangents.Add(Out.Tangents.IsValidIndex(o[i]) ? Out.Tangents[o[i]] : FProcMeshTangent());
            }
            r[i] = idx;
        }
        New.Triangles.Append({ r[0], r[1], r[2] });
    }

    Out = MoveTemp(New);
}

// ============================================================
// Actor
// ============================================================

AMountainGenWorldActor::AMountainGenWorldActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);

    ProcMesh->SetMobility(EComponentMobility::Movable);
    ProcMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    ProcMesh->bUseComplexAsSimpleCollision = true;
    ProcMesh->bUseAsyncCooking = true;

    AutoReceiveInput = EAutoReceiveInput::Player0;
}

void AMountainGenWorldActor::UI_Status(const FString& Msg, float Seconds, FColor Color) const
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, Seconds, Color, Msg);
}

static bool MG_InRange(float V, float Min, float Max)
{
    return (V >= Min && V <= Max);
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

#if WITH_EDITOR
uint32 AMountainGenWorldActor::ComputeSettingsHash_Editor() const
{
    uint32 H = 0;

    H = HashCombine(H, ::GetTypeHash(Settings.Seed));
    H = HashCombine(H, ::GetTypeHash((uint8)Settings.Difficulty));
    H = HashCombine(H, ::GetTypeHash(Settings.bAutoTune));
    H = HashCombine(H, ::GetTypeHash(Settings.SeedSearchTries));
    H = HashCombine(H, ::GetTypeHash(Settings.bRetrySeedUntilSatisfied));
    H = HashCombine(H, ::GetTypeHash(Settings.MaxSeedAttempts));

    H = HashCombine(H, ::GetTypeHash(Settings.VoxelSizeCm));
    H = HashCombine(H, ::GetTypeHash(Settings.IsoLevel));

    H = HashCombine(H, ::GetTypeHash(Settings.CliffThicknessCm));
    H = HashCombine(H, ::GetTypeHash(Settings.CliffDepthCm));
    H = HashCombine(H, ::GetTypeHash(Settings.CliffHalfWidthCm));
    H = HashCombine(H, ::GetTypeHash(Settings.CliffHeightCm));
    H = HashCombine(H, ::GetTypeHash(Settings.BaseHeightCm));

    H = HashCombine(H, ::GetTypeHash(Settings.bCreateCollision));

    return H;
}
#endif

void AMountainGenWorldActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UWorld* W = GetWorld();

    if (W && W->IsGameWorld())
        return;

#if WITH_EDITOR
    const uint32 NewHash = ComputeSettingsHash_Editor();
    const bool bFirst = (LastSettingsHash_Editor == 0);
    const bool bSettingsChanged = (!bFirst && NewHash != LastSettingsHash_Editor);

    if (bFirst || bSettingsChanged)
    {
        LastSettingsHash_Editor = NewHash;
        BuildChunkAndMesh();
    }
    return;
#else
    BuildChunkAndMesh();
#endif
}

#if WITH_EDITOR
void AMountainGenWorldActor::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    (void)bFinished;
}

void AMountainGenWorldActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (!PropertyChangedEvent.Property) return;

    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
    BuildChunkAndMesh();
}
#endif

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

    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AMountainGenWorldActor::CycleDifficulty);
    InputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &AMountainGenWorldActor::CycleDifficulty);
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
        Result.FinalSettings.bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
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
    const int32 Clamped = FMath::Max(1, NewSeed);

    if (bAsyncWorking)
    {
        UI_Status(TEXT("[MountainGen] 작업 중... 완료 후 다시 시도"), 1.2f, FColor::Red);
        return;
    }

    if (Settings.Seed == Clamped) return;

    Settings.Seed = Clamped;

    UI_Status(FString::Printf(TEXT("[MountainGen] 시드 변경 요청: %d"), Settings.Seed), 1.5f, FColor::Cyan);

#if WITH_EDITOR
    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
#endif

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

#if WITH_EDITOR
    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
#endif

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::CycleDifficulty()
{
    using ED = EMountainGenDifficulty;

    switch (Settings.Difficulty)
    {
    case ED::Easy:   Settings.Difficulty = ED::Normal; break;
    case ED::Normal: Settings.Difficulty = ED::Hard;   break;
    case ED::Hard:   Settings.Difficulty = ED::Easy;   break;
    default:         Settings.Difficulty = ED::Easy;   break;
    }

    FString DifficultyText;
    switch (Settings.Difficulty)
    {
    case ED::Easy:   DifficultyText = TEXT("Easy"); break;
    case ED::Normal: DifficultyText = TEXT("Normal"); break;
    case ED::Hard:   DifficultyText = TEXT("Hard"); break;
    default:         DifficultyText = TEXT("Unknown"); break;
    }

    UI_Status(
        FString::Printf(TEXT("[MountainGen] 난이도 변경 → %s"), *DifficultyText),
        2.0f,
        FColor::Green
    );

#if WITH_EDITOR
    LastSettingsHash_Editor = ComputeSettingsHash_Editor();
#endif

    BuildChunkAndMesh();
}

void AMountainGenWorldActor::BuildChunkAndMesh()
{
    if (!ProcMesh) return;

    UWorld* W = GetWorld();
    const bool bEditorLike = (!W || !W->IsGameWorld());

    auto DebugPrintGT = [this](const FString& Msg, float Sec, FColor Col)
        {
            if (IsInGameThread())
            {
                UI_Status(Msg, Sec, Col);
            }
            else
            {
                TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);
                AsyncTask(ENamedThreads::GameThread, [WeakThis, Msg, Sec, Col]()
                    {
                        if (!WeakThis.IsValid()) return;
                        WeakThis->UI_Status(Msg, Sec, Col);
                    });
            }
        };

    auto PrintMetrics = [&](const TCHAR* Prefix, const FMountainGenSettings& SS, const FMGMetrics& MM, float Sec, FColor Col)
        {
            bool okO = false, okS = false;
            const FString Line = FString::Printf(TEXT("%s %s"), Prefix, *MakeMetricsLine(SS, MM, okO, okS));
            DebugPrintGT(Line, Sec, Col);
        };

    // =========================================================
    // 0) Effective Settings
    // =========================================================
    FMountainGenSettings S = Settings;
    MGApplyDifficultyPreset(S);

    const float Voxel = FMath::Max(1.f, S.VoxelSizeCm);
    const FVector ActorWorld = GetActorLocation();

    const float FrontX = FMath::Max(200.f, S.CliffThicknessCm);
    const FVector TerrainOriginWorld = ActorWorld - FVector(FrontX, 0.f, 0.f);

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

    if (S.MetricsStepCm <= 0.f)
        S.MetricsStepCm = FMath::Max(400.f, S.VoxelSizeCm * 2.f);

    const int32 InputSeed = S.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, S.SeedSearchTries);

    if (bDebugPipeline)
    {
        DebugPrintGT(
            FString::Printf(TEXT("[MountainGen] PATH=%s  SeedSearchTries=%d  Sample=%dx%dx%d"),
                bEditorLike ? TEXT("EditorLike") : TEXT("RuntimeAsync"),
                TriesForSeedSearch, SampleX, SampleY, SampleZ),
            4.0f,
            bEditorLike ? FColor::Yellow : FColor::Green
        );
    }

    // =========================================================
    // (A) Editor: 동기 생성
    // =========================================================
    if (bEditorLike)
    {
        if (bDebugPipeline)
        {
            const FMGMetrics M0 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
            PrintMetrics(TEXT("[AutoTune][Before]"), S, M0, 5.0f, FColor::Cyan);
        }

        if (S.bAutoTune)
        {
            MGAutoTuneIntentParams(S, TerrainOriginWorld, WorldMin, WorldMax);
            MGClampToDifficultyBounds(S);
        }

        if (bDebugPipeline)
        {
            const FMGMetrics M1 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
            PrintMetrics(TEXT("[AutoTune][After ]"), S, M1, 5.0f, FColor::Cyan);
        }

        auto DebugPrint = [DebugPrintGT](const FString& Msg, float Sec, FColor Col)
            {
                DebugPrintGT(Msg, Sec, Col);
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

        if (bDebugPipeline)
        {
            const FMGMetrics MF = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
            bool okO = false, okS = false;
            const FString Line =
                FString::Printf(TEXT("[FinalMetrics] seed=%d | %s"), S.Seed, *MakeMetricsLine(S, MF, okO, okS));
            DebugPrintGT(Line, 6.0f, (okO && okS) ? FColor::Green : FColor::Orange);
        }

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

        if (bDebugPipeline)
        {
            DebugPrintGT(
                FString::Printf(TEXT("[Mesh][Raw ] V=%d  T=%d"),
                    MeshData.Vertices.Num(),
                    MeshData.Triangles.Num() / 3),
                4.0f,
                FColor::Silver
            );
        }

        MG_WeldVertices_Quantized(MeshData, S.VoxelSizeCm * 0.15f);

        if (bDebugPipeline)
        {
            DebugPrintGT(
                FString::Printf(TEXT("[Mesh][Weld] V=%d  T=%d"),
                    MeshData.Vertices.Num(),
                    MeshData.Triangles.Num() / 3),
                4.0f,
                FColor::Silver
            );
        }

        MG_CullMeshIslands(MeshData, 200, true);

        if (bDebugPipeline)
        {
            DebugPrintGT(
                FString::Printf(TEXT("[Mesh][Cull] V=%d  T=%d"),
                    MeshData.Vertices.Num(),
                    MeshData.Triangles.Num() / 3),
                4.0f,
                FColor::Silver
            );
        }

        ProcMesh->ClearAllMeshSections();
        ProcMesh->ClearCollisionConvexMeshes();

        if (MeshData.Vertices.Num() == 0 || MeshData.Triangles.Num() == 0)
        {
            DebugPrintGT(TEXT("[MountainGen][Editor] MeshData 비어있음 (Cull/Weld로 모두 제거됨)"), 2.0f, FColor::Red);
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

        Settings.Seed = S.Seed;

        if (bDebugPipeline)
            DebugPrintGT(FString::Printf(TEXT("[MountainGen][Editor] DONE Seed=%d"), Settings.Seed), 2.0f, FColor::Green);

        return;
    }

    // =========================================================
 // (B) Runtime: 비동기
 // =========================================================
    if (bAsyncWorking)
    {
        bRegenQueued = true;
        DebugPrintGT(TEXT("[MountainGen] 작업 중 → 재생성 큐"), 1.5f, FColor::Orange);
        return;
    }

    const int32 LocalBuildSerial = ++CurrentBuildSerial;
    bAsyncWorking = true;
    InFlightBuildSerial = LocalBuildSerial;

    if (bDebugPipeline)
        DebugPrintGT(TEXT("[MountainGen] AutoTune/SeedSearch/생성 시작"), 2.0f, FColor::Cyan);

    TWeakObjectPtr<AMountainGenWorldActor> WeakThis(this);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
        [WeakThis,
        S, TerrainOriginWorld,
        WorldMin, WorldMax,
        ChunkOriginWorld, ActorWorld, SampleOriginWorld,
        SampleX, SampleY, SampleZ, Voxel,
        InputSeed, TriesForSeedSearch, LocalBuildSerial]() mutable
        {
            if (!WeakThis.IsValid()) return;

            auto DebugPrint = [WeakThis](const FString& Msg, float Sec, FColor Col)
                {
                    AsyncTask(ENamedThreads::GameThread, [WeakThis, Msg, Sec, Col]()
                        {
                            if (!WeakThis.IsValid()) return;
                            WeakThis->UI_Status(Msg, Sec, Col);
                        });
                };

            // ---- AutoTune ----
            if (WeakThis->bDebugPipeline)
            {
                const FMGMetrics M0 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
                bool okO = false, okS = false;
                DebugPrint(TEXT("[AutoTune][Before] ") + WeakThis->MakeMetricsLine(S, M0, okO, okS), 5.0f, FColor::Cyan);
            }

            if (S.bAutoTune)
            {
                MGAutoTuneIntentParams(S, TerrainOriginWorld, WorldMin, WorldMax);
                MGClampToDifficultyBounds(S);
            }

            if (WeakThis->bDebugPipeline)
            {
                const FMGMetrics M1 = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
                bool okO = false, okS = false;
                DebugPrint(TEXT("[AutoTune][After ] ") + WeakThis->MakeMetricsLine(S, M1, okO, okS), 5.0f, FColor::Cyan);
            }

            // ---- Seed Search ----
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

            if (WeakThis->bDebugPipeline)
            {
                const FMGMetrics MF = MGComputeMetricsQuick(S, TerrainOriginWorld, WorldMin, WorldMax);
                bool okO = false, okS = false;
                const FString Line =
                    FString::Printf(TEXT("[FinalMetrics] seed=%d | %s"), S.Seed, *WeakThis->MakeMetricsLine(S, MF, okO, okS));
                DebugPrint(Line, 6.0f, (okO && okS) ? FColor::Green : FColor::Orange);
            }

            // ---- Density sampling ----
            FVoxelChunk Chunk;
            Chunk.Init(SampleX, SampleY, SampleZ);

            FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

            // precompute offsets
            TArray<float> XOff; XOff.SetNumUninitialized(SampleX);
            TArray<float> YOff; YOff.SetNumUninitialized(SampleY);
            TArray<float> ZOff; ZOff.SetNumUninitialized(SampleZ);

            for (int32 x = 0; x < SampleX; ++x) XOff[x] = x * Voxel;
            for (int32 y = 0; y < SampleY; ++y) YOff[y] = y * Voxel;
            for (int32 z = 0; z < SampleZ; ++z) ZOff[z] = z * Voxel;

            ParallelFor(SampleZ, [&](int32 z)
                {
                    const float zc = ZOff[z];
                    for (int32 y = 0; y < SampleY; ++y)
                    {
                        const float yc = YOff[y];
                        for (int32 x = 0; x < SampleX; ++x)
                        {
                            const FVector WorldPos = SampleOriginWorld + FVector(XOff[x], yc, zc);
                            Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                        }
                    }
                }, EParallelForFlags::Unbalanced);

            // ---- Meshing / Weld / Cull ----
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

            if (WeakThis->bDebugPipeline)
            {
                DebugPrint(
                    FString::Printf(TEXT("[Mesh][Raw ] V=%d  T=%d"),
                        MeshData.Vertices.Num(),
                        MeshData.Triangles.Num() / 3),
                    4.0f,
                    FColor::Silver
                );
            }

            MG_WeldVertices_Quantized(MeshData, S.VoxelSizeCm * 0.15f);

            if (WeakThis->bDebugPipeline)
            {
                DebugPrint(
                    FString::Printf(TEXT("[Mesh][Weld] V=%d  T=%d"),
                        MeshData.Vertices.Num(),
                        MeshData.Triangles.Num() / 3),
                    4.0f,
                    FColor::Silver
                );
            }

            MG_CullMeshIslands(MeshData, 200, true);

            if (WeakThis->bDebugPipeline)
            {
                DebugPrint(
                    FString::Printf(TEXT("[Mesh][Cull] V=%d  T=%d"),
                        MeshData.Vertices.Num(),
                        MeshData.Triangles.Num() / 3),
                    4.0f,
                    FColor::Silver
                );
            }

            // ---- GameThread apply ----
            AsyncTask(ENamedThreads::GameThread,
                [WeakThis, FinalS = S, MeshData = MoveTemp(MeshData), LocalBuildSerial]() mutable
                {
                    if (!WeakThis.IsValid()) return;
                    if (WeakThis->InFlightBuildSerial != LocalBuildSerial) return;

                    WeakThis->PendingResult.bValid = true;
                    WeakThis->PendingResult.BuildSerial = LocalBuildSerial;
                    WeakThis->PendingResult.FinalSettings = FinalS;
                    WeakThis->PendingResult.MeshData = MoveTemp(MeshData);

                    if (WeakThis->bDebugPipeline)
                    {
                        WeakThis->UI_Status(
                            FString::Printf(TEXT("[MountainGen] 시드 확정: %d"), FinalS.Seed),
                            2.5f,
                            FColor::Green
                        );
                    }
                });
        });
}