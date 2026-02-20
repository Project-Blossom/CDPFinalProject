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
    // 난이도별로 "지형 성격"도 바뀌게 한다.
    // 핵심: Overhang/Steep에 영향 큰 것들
    // - BaseField3DStrengthCm (3D 덩어리감/블롭/언더컷 성향)
    // - BaseField3DScaleCm    (큰/작은 형태의 스케일)
    // - DetailScaleCm         (미세 굴곡 -> 경사/거칠기 증가)
    // - VolumeStrength        (오버행 강도)
    // - OverhangScaleCm       (오버행 노이즈 스케일)
    // - OverhangFadeCm        (오버행이 위/아래로 얼마나 유지되는지)
    // - OverhangBias          (오버행 발생 문턱)
    // - GravityStrength/Scale (중력 shaping: 수직벽/언더컷 억제/강화)
    //
    // ※ VoxelSizeCm까지 난이도에서 바꾸면 메트릭이 크게 튀고 성능도 달라지니
    //   일단 고정 추천. (원하면 나중에 난이도별 LOD처럼 설계)

    switch (S.Difficulty)
    {
    case EMountainGenDifficulty::Easy:
        // ---------------- targets(검사 기준) ----------------
        S.Targets.OverhangMin = 0.00f; S.Targets.OverhangMax = 0.05f;
        S.Targets.SteepMin = 0.05f; S.Targets.SteepMax = 0.20f;

        // ---------------- shape weights(생성 가중치) ----------------
        S.BaseField3DStrengthCm = 5000.f;   // 12000 -> 감소 (블롭/언더컷 감소)
        S.BaseField3DScaleCm = 22000.f;  // 크게 -> 완만
        S.DetailScaleCm = 9000.f;   // 디테일 완만
        S.VolumeStrength = 0.20f;    // 오버행 거의 끔
        S.OverhangScaleCm = 14000.f;  // 큰 스케일 -> 급격한 오버행 줄음
        S.OverhangFadeCm = 28000.f;  // 더 빨리 사라지게(위/아래 영향 완화)
        S.OverhangBias = 0.70f;    // 오버행 발생 어렵게
        S.GravityStrength = 1.30f;    // 중력 shaping 강화(언더컷 억제)
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

        // 기본값에 가깝게
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

        S.BaseField3DStrengthCm = 17000.f;  // 더 거칠고 3D 강해짐
        S.BaseField3DScaleCm = 12000.f;  // 스케일 줄여서 급격
        S.DetailScaleCm = 4200.f;   // 디테일 더 촘촘 -> 경사 증가
        S.VolumeStrength = 1.60f;    // 오버행 확실히 증가
        S.OverhangScaleCm = 6000.f;   // 더 촘촘한 오버행
        S.OverhangFadeCm = 9000.f;   // 오버행 영향 더 강하게 유지
        S.OverhangBias = 0.48f;    // 오버행 쉽게
        S.GravityStrength = 0.85f;    // 중력 shaping 약하게(언더컷 허용)
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

    ApplyDifficultyPreset();

    FMountainGenSettings S = Settings;
    ApplyDifficultyPresetTo(S);

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
        const float Band = FMath::Max(
            (S.FrontBandDepthCm > 0.f) ? S.FrontBandDepthCm : S.CliffDepthCm,
            Voxel * 2.f
        );

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

    const FVector MetricsWorldMin =
        TerrainOriginWorld + FVector(
            -S.ChunkX * Voxel * 0.5f,
            -S.ChunkY * Voxel * 0.5f,
            S.BaseHeightCm
        );

    const FVector MetricsWorldMax =
        TerrainOriginWorld + FVector(
            S.ChunkX * Voxel * 0.5f,
            S.ChunkY * Voxel * 0.5f,
            S.BaseHeightCm + S.ChunkZ * Voxel
        );

    const int32 InputSeed = S.Seed;
    const int32 TriesForSeedSearch = FMath::Max(1, S.SeedSearchTries);

    UI_Status(
        FString::Printf(TEXT("[MountainGen] PATH=%s  SeedSearchTries=%d  Sample=%dx%dx%d"),
            bEditorLike ? TEXT("EditorLike") : TEXT("RuntimeAsync"),
            TriesForSeedSearch, SampleX, SampleY, SampleZ),
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

        if (S.Seed <= 0)
        {
            const int32 Hash = (int32)((PTRINT)this) ^ (int32)ActorWorld.X ^ ((int32)ActorWorld.Y << 1) ^ ((int32)ActorWorld.Z << 2);
            FRandomStream Rng(Hash ^ 0x51A3B9D1);
            S.Seed = Rng.RandRange(1, INT32_MAX);
        }

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

            (void)MGTuneSettingsFeedback(S, TerrainOriginWorld, WorldMin, WorldMax);
        }

        Settings.Seed = S.Seed;

        {
            const FMGMetrics EM = MGComputeMetricsQuick(S, TerrainOriginWorld, MetricsWorldMin, MetricsWorldMax);

            bool bO = false, bSt = false;
            const FString Line = MakeMetricsLine(S, EM, bO, bSt);

            UI_Status(
                FString::Printf(TEXT("[MountainGen][EditorMetrics] seed=%d | %s"), S.Seed, *Line),
                6.0f,
                (bO && bSt) ? FColor::Green : FColor::Orange
            );
        }

        FVoxelChunk Chunk;
        Chunk.Init(SampleX, SampleY, SampleZ);

        FVoxelDensityGenerator Gen(S, TerrainOriginWorld);

        ParallelFor(SampleZ, [&](int32 z)
            {
                for (int32 y = 0; y < SampleY; ++y)
                {
                    for (int32 x = 0; x < SampleX; ++x)
                    {
                        const FVector WorldPos = SampleOriginWorld + FVector(x * Voxel, y * Voxel, z * Voxel);
                        Chunk.Set(x, y, z, Gen.SampleDensity(WorldPos));
                    }
                }
            });

        MG_CarveCaves_PostProcess(Chunk, S, SampleOriginWorld, Voxel);

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

        UI_Status(FString::Printf(TEXT("[MountainGen][Editor] Seed=%d (FrontBand)"), Settings.Seed), 2.0f, FColor::Green);
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
        S, TerrainOriginWorld,
        WorldMin, WorldMax,
        MetricsWorldMin, MetricsWorldMax,
        ChunkOriginWorld, ActorWorld, SampleOriginWorld,
        SampleX, SampleY, SampleZ, Voxel,
        InputSeed, TriesForSeedSearch, LocalBuildSerial]() mutable
        {
            if (!WeakThis.IsValid()) return;

            FMountainGenSettings BaseS = S;

            FRandomStream SeedRng;
            if (InputSeed > 0)
            {
                SeedRng.Initialize(InputSeed ^ 0x1F3A9B2D);
            }
            else
            {
                const uint64 T = FPlatformTime::Cycles64();
                SeedRng.Initialize(
                    (int32)(T & 0x7fffffff) ^
                    (int32)((T >> 32) & 0x7fffffff) ^
                    (LocalBuildSerial * 977) ^
                    (int32)FMath::RoundToInt(SampleOriginWorld.X) ^
                    ((int32)FMath::RoundToInt(SampleOriginWorld.Y) << 7) ^
                    ((int32)FMath::RoundToInt(SampleOriginWorld.Z) << 14)
                );
            }

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

            const int32 MaxSeedTries =
                BaseS.bRetrySeedUntilSatisfied
                ? FMath::Max(1, BaseS.MaxSeedAttempts)
                : FMath::Max(1, TriesForSeedSearch);

            for (int32 Attempt = 1; Attempt <= MaxSeedTries; ++Attempt)
            {
                const int32 CandSeed =
                    (Attempt == 1 && InputSeed > 0)
                    ? InputSeed
                    : SeedRng.RandRange(1, INT32_MAX);

                FMountainGenSettings Cand = BaseS;
                Cand.Seed = CandSeed;

                if (Cand.bAutoTune)
                {
                    const float FixedHeightAmp = Cand.HeightAmpCm;
                    const float FixedRadius = Cand.EnvelopeRadiusCm;
                    const float FixedBaseH = Cand.BaseHeightCm;

                    MGDeriveParamsFromSeed(Cand, CandSeed);
                    (void)MGFinalizeSettingsFromSeed(Cand, TerrainOriginWorld, WorldMin, WorldMax);

                    Cand.HeightAmpCm = FixedHeightAmp;
                    Cand.EnvelopeRadiusCm = FixedRadius;
                    Cand.BaseHeightCm = FixedBaseH;
                }

                const FMGMetrics M = MGComputeMetricsQuick(Cand, TerrainOriginWorld, MetricsWorldMin, MetricsWorldMax);

                bool bOverOK = false;
                bool bSteepOK = false;
                const FString Line = WeakThis->MakeMetricsLine(Cand, M, bOverOK, bSteepOK);

                if (bOverOK && bSteepOK)
                {
                    FinalSeed = CandSeed;
                    bSatisfied = true;

                    if (WeakThis->bDebugSeedSearch)
                    {
                        AsyncTask(ENamedThreads::GameThread,
                            [WeakThis, Attempt, CandSeed, Line, MaxSeedTries]()
                            {
                                if (!WeakThis.IsValid()) return;

                                WeakThis->UI_Status(
                                    FString::Printf(TEXT("[SeedSearch] SATISFIED %d/%d seed=%d | %s"),
                                        Attempt, MaxSeedTries, CandSeed, *Line),
                                    6.0f,
                                    FColor::Green
                                );
                            });
                    }
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
                    const bool bPrint =
                        (Attempt == 1) ||
                        (Attempt == MaxSeedTries) ||
                        ((Attempt % EveryN) == 0);

                    if (bPrint)
                    {
                        FColor FailColor = FColor::Red;
                        if (!bOverOK && bSteepOK)      FailColor = FColor::Blue;
                        else if (bOverOK && !bSteepOK) FailColor = FColor::Yellow;

                        AsyncTask(ENamedThreads::GameThread,
                            [WeakThis, Attempt, CandSeed, Line, FailColor, MaxSeedTries]()
                            {
                                if (!WeakThis.IsValid()) return;

                                WeakThis->UI_Status(
                                    FString::Printf(TEXT("[SeedSearch] FAIL %d/%d seed=%d | %s"),
                                        Attempt, MaxSeedTries, CandSeed, *Line),
                                    4.5f,
                                    FailColor
                                );
                            });
                    }
                }
            }

            if (!bSatisfied)
            {
                FinalSeed = BestSeed;

                FMountainGenSettings BestCand = BaseS;
                BestCand.Seed = BestSeed;

                if (BestCand.bAutoTune)
                {
                    const float FixedHeightAmp = BestCand.HeightAmpCm;
                    const float FixedRadius = BestCand.EnvelopeRadiusCm;
                    const float FixedBaseH = BestCand.BaseHeightCm;

                    MGDeriveParamsFromSeed(BestCand, BestSeed);
                    (void)MGFinalizeSettingsFromSeed(BestCand, TerrainOriginWorld, WorldMin, WorldMax);

                    BestCand.HeightAmpCm = FixedHeightAmp;
                    BestCand.EnvelopeRadiusCm = FixedRadius;
                    BestCand.BaseHeightCm = FixedBaseH;
                }

                const FMGMetrics BestM = MGComputeMetricsQuick(BestCand, TerrainOriginWorld, MetricsWorldMin, MetricsWorldMax);

                bool bOverOK = false;
                bool bSteepOK = false;
                const FString BestLine = WeakThis->MakeMetricsLine(BestCand, BestM, bOverOK, bSteepOK);

                FColor FailColor = FColor::Red;
                if (!bOverOK && bSteepOK)      FailColor = FColor::Blue;
                else if (bOverOK && !bSteepOK) FailColor = FColor::Yellow;

                AsyncTask(ENamedThreads::GameThread,
                    [WeakThis, FinalSeed, BestLine, FailColor]()
                    {
                        if (!WeakThis.IsValid()) return;

                        WeakThis->UI_Status(
                            FString::Printf(TEXT("[SeedSearch] ALL FAILED -> fallback seed=%d | %s"), FinalSeed, *BestLine),
                            6.0f,
                            FailColor
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

                (void)MGTuneSettingsFeedback(FinalS, TerrainOriginWorld, WorldMin, WorldMax);
            }

            // (3) Mesh Build
            FVoxelChunk Chunk;
            Chunk.Init(SampleX, SampleY, SampleZ);

            FVoxelDensityGenerator Gen(FinalS, TerrainOriginWorld);

            ParallelFor(SampleZ, [&](int32 z)
                {
                    for (int32 y = 0; y < SampleY; ++y)
                    {
                        for (int32 x = 0; x < SampleX; ++x)
                        {
                            const float wx = WorldMin.X + float(x) * Voxel;
                            const float wy = WorldMin.Y + float(y) * Voxel;
                            const float wz = WorldMin.Z + float(z) * Voxel;

                            const FVector WorldPos(wx, wy, wz);
                            const float D = Gen.SampleDensity(WorldPos);

                            Chunk.Set(x, y, z, D);
                        }
                    }
                });

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
                        FString::Printf(TEXT("[MountainGen] 시드 확정: %d"), FinalS.Seed),
                        2.5f,
                        FColor::Green
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