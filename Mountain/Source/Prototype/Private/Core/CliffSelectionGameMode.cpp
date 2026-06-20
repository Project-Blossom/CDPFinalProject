// File: Source/Prototype/Private/Core/CliffSelectionGameMode.cpp
#include "Core/CliffSelectionGameMode.h"
#include "Core/DownfallGameInstance.h"
#include "UI/CliffSelectionLoadingWidget.h"
#include "MountainGenWorldActor.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

ACliffSelectionGameMode::ACliffSelectionGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
}

EMountainGenDifficulty ACliffSelectionGameMode::ResolveDifficultyFromStageIndex() const
{
    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (!GI)
    {
        UE_LOG(LogTemp, Warning, TEXT("CliffSelectionGameMode: GameInstance not found, defaulting to Normal"));
        return EMountainGenDifficulty::Normal;
    }

    const int32 StageIndex = GI->GetCurrentStageIndex();

    // 기획: Stage1 클리어 후(CurrentStageIndex==1) -> Medium(Normal), Stage2 클리어 후(==2) -> Hard
    switch (StageIndex)
    {
    case 2:
        return EMountainGenDifficulty::Hard;
    case 1:
    default:
        return EMountainGenDifficulty::Normal;
    }
}

void ACliffSelectionGameMode::BeginPlay()
{
    Super::BeginPlay();

    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (GI)
    {
        GI->GenerateNewSeeds();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionGameMode: GameInstance is not UDownfallGameInstance"));
    }

    // 로딩 위젯은 Pawn::BeginPlay에서 생성 (PC 확보 시점)
    // GameMode BeginPlay에서는 암벽만 스폰
    SpawnCliffs();
}

void ACliffSelectionGameMode::ShowLoadingWidget()
{
    if (IsValid(CurrentLoadingWidget))
    {
        // 이미 표시 중
        return;
    }

    if (!LoadingWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("CliffSelectionGameMode: LoadingWidgetClass not set"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    CurrentLoadingWidget = CreateWidget<UCliffSelectionLoadingWidget>(PC, LoadingWidgetClass);
    if (CurrentLoadingWidget)
    {
        CurrentLoadingWidget->AddToViewport();
    }
}


void ACliffSelectionGameMode::SpawnCliffs()
{
    if (!CliffActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionGameMode: CliffActorClass not set (BP defaults)"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    const TArray<int32>& Seeds = GI ? GI->GetGeneratedSeeds() : TArray<int32>();

    if (Seeds.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionGameMode: GeneratedSeeds count < 3 (%d)"), Seeds.Num());
        return;
    }

    const EMountainGenDifficulty Difficulty = ResolveDifficultyFromStageIndex();

    FVector CenterLocation = FVector::ZeroVector;
    if (AActor* PlayerStart = FindPlayerStart(nullptr))
    {
        CenterLocation = PlayerStart->GetActorLocation();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CliffSelectionGameMode: PlayerStart not found, using ZeroVector as center"));
    }

    const int32 SpawnCount = FMath::Min(3, CliffAnglesDeg.Num());

    CompletedCliffCount = 0;
    ExpectedCliffCount = SpawnCount;
    SpawnedCliffs.Reset();
    SpawnedCliffs.Reserve(SpawnCount);

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        const float AngleDeg = CliffAnglesDeg[i];
        const float AngleRad = FMath::DegreesToRadians(AngleDeg);

        const FVector SpawnOffset(
            FMath::Cos(AngleRad) * SpawnRadiusCm,
            FMath::Sin(AngleRad) * SpawnRadiusCm,
            0.f
        );
        const FVector SpawnLocation = CenterLocation + SpawnOffset;

        // 카메라 중심을 바라보도록 회전 (암벽 정면이 CenterLocation 방향)
        const FRotator SpawnRotation = (CenterLocation - SpawnLocation).Rotation();

        AMountainGenWorldActor* NewCliff = World->SpawnActorDeferred<AMountainGenWorldActor>(
            CliffActorClass,
            FTransform(SpawnRotation, SpawnLocation),
            nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        if (!NewCliff)
        {
            UE_LOG(LogTemp, Error, TEXT("CliffSelectionGameMode: Failed to spawn cliff %d"), i);
            continue;
        }

        // OnConstruction/BeginPlay 전에 Settings 적용
        NewCliff->Settings.Seed = FMath::Max(1, Seeds[i]);
        NewCliff->Settings.Difficulty = Difficulty;

        NewCliff->OnMountainGenerated.AddDynamic(this, &ACliffSelectionGameMode::HandleCliffGenerated);

        // FinishSpawningActor가 BeginPlay를 동기 호출하며, 그 안에서 BuildChunkAndMesh가
        // 동기적으로 완료되어 OnMountainGenerated가 즉시 브로드캐스트될 수 있다.
        // 따라서 콜백이 SpawnedCliffs.Num()을 올바르게 참조하도록 Add를 먼저 수행한다.
        SpawnedCliffs.Add(NewCliff);

        UGameplayStatics::FinishSpawningActor(NewCliff, FTransform(SpawnRotation, SpawnLocation));

        // 새로 스폰된 ProcMesh는 섹션이 없어도 Bounds가 valid로 평가되어
        // BeginPlay에서 BuildChunkAndMesh가 호출되지 않는 경우가 있다 (UE5.7 빈 ProcMesh 기본 Bounds 이슈).
        // 항상 강제로 재생성을 요청하여 BuildChunkAndMesh가 실제로 실행되도록 한다.
        NewCliff->Regenerate();
    }

    UE_LOG(LogTemp, Log, TEXT("CliffSelectionGameMode: Spawned %d cliffs (Difficulty=%d, Seeds=%d,%d,%d)"),
        SpawnedCliffs.Num(), (int32)Difficulty, Seeds[0], Seeds[1], Seeds[2]);
}

void ACliffSelectionGameMode::DestroySpawnedCliffs()
{
    for (AMountainGenWorldActor* Cliff : SpawnedCliffs)
    {
        if (IsValid(Cliff))
        {
            Cliff->OnMountainGenerated.RemoveDynamic(this, &ACliffSelectionGameMode::HandleCliffGenerated);
            Cliff->Destroy();
        }
    }
    SpawnedCliffs.Reset();
    CompletedCliffCount = 0;
}

void ACliffSelectionGameMode::HandleCliffGenerated(AActor* Generator)
{
    // BeginPlay 시점의 빈 메시 브로드캐스트(bHasExistingSection=0, bHasValidBounds=1 경로)를 무시하고
    // Regenerate()로 인한 실제 메시 완료 브로드캐스트만 카운트한다.
    // 판별 기준: Generator가 AMountainGenWorldActor이고 HasGeneratedMesh()가 true인 경우만 유효
    AMountainGenWorldActor* Cliff = Cast<AMountainGenWorldActor>(Generator);
    if (!Cliff || !Cliff->HasGeneratedMesh())
    {
        UE_LOG(LogTemp, Log, TEXT("CliffSelectionGameMode: Cliff broadcast ignored (empty mesh) - %s"),
            Generator ? *Generator->GetName() : TEXT("Unknown"));
        return;
    }

    ++CompletedCliffCount;

    UE_LOG(LogTemp, Log, TEXT("CliffSelectionGameMode: Cliff generated (%d/%d) - %s"),
        CompletedCliffCount, ExpectedCliffCount,
        *Generator->GetName());

    if (CompletedCliffCount >= ExpectedCliffCount)
    {
        OnAllCliffsGenerated.Broadcast();
    }
}

/*
// [DEAD CODE] 실제 선택 확정 흐름은 ACliffSelectionPawn::OnConfirmSelection에서 처리됨.
// 이 GameMode 버전은 어디서도 호출되지 않아 주석 처리함.
void ACliffSelectionGameMode::OnConfirmSelection(int32 SelectedCliffIndex)
{
    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelectionGameMode::OnConfirmSelection: GameInstance not found"));
        return;
    }

    // 선택된 암벽의 Seed를 GameInstance에 저장
    const TArray<int32>& Seeds = GI->GetGeneratedSeeds();
    if (Seeds.IsValidIndex(SelectedCliffIndex))
    {
        GI->SetSelectedSeed(Seeds[SelectedCliffIndex]);
        UE_LOG(LogTemp, Warning, TEXT("CliffSelection: Selected Seed=%d (Index=%d)"),
            Seeds[SelectedCliffIndex], SelectedCliffIndex);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CliffSelection: Invalid SelectedCliffIndex=%d"), SelectedCliffIndex);
        return;
    }

    // CurrentStageIndex 증가 (CliffSelection 완료 → 다음 스테이지로)
    const int32 NextStageIndex = GI->GetCurrentStageIndex() + 1;
    GI->SetCurrentStageIndex(NextStageIndex);

    // Loading UI 표시 플래그 설정
    GI->SetShowLoadingUI(true);

    // CurrentStageIndex 기준 다음 레벨 결정
    // Stage_2 → Stage_3 순서
    FName NextLevel;
    if (NextStageIndex == 2)
    {
        NextLevel = FName("Stage_2");
    }
    else if (NextStageIndex == 3)
    {
        NextLevel = FName("Stage_3");
    }
    else
    {
        // Ending 미구현 — 현재는 Stage_3 유지
        UE_LOG(LogTemp, Warning, TEXT("CliffSelection: StageIndex=%d, Ending not implemented yet"), NextStageIndex);
        NextLevel = FName("Stage_3");
    }

    UE_LOG(LogTemp, Warning, TEXT("CliffSelection: → OpenLevel(%s) [StageIndex=%d]"),
        *NextLevel.ToString(), NextStageIndex);

    UGameplayStatics::OpenLevel(this, NextLevel);
}
*/

void ACliffSelectionGameMode::RerollCliffs()
{
    if (bRerollUsed)
    {
        UE_LOG(LogTemp, Warning, TEXT("CliffSelectionGameMode: Reroll already used"));
        return;
    }

    bRerollUsed = true;

    UDownfallGameInstance* GI = GetGameInstance<UDownfallGameInstance>();
    if (GI)
    {
        GI->GenerateNewSeeds();
    }

    DestroySpawnedCliffs();
    ShowLoadingWidget();
    SpawnCliffs();
}
