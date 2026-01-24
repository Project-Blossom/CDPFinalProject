#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MountainGenSettings.h"
#include "MountainGenWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class MOUNTAINGEN_API AMountainGenWorldActor : public AActor
{
    GENERATED_BODY()

public:
    AMountainGenWorldActor();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void Regenerate();

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void SetSeed(int32 NewSeed);

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void RandomizeSeed();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen")
    TObjectPtr<UProceduralMeshComponent> ProcMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Mesh")
    TObjectPtr<UMaterialInterface> VoxelMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen")
    FMountainGenSettings Settings;

    // 런타임에서 1번 키로 랜덤 시드 재생성을 할지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Runtime")
    bool bEnableRandomSeedKey = true;

private:
    void BuildChunkAndMesh();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};