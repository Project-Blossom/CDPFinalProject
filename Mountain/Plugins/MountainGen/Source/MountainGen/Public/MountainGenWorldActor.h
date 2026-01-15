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

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void Regenerate();

    UFUNCTION(BlueprintCallable, Category = "MountainGen")
    void SetSeed(int32 NewSeed);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen")
    TObjectPtr<UProceduralMeshComponent> ProcMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen|Mesh")
    TObjectPtr<UMaterialInterface> VoxelMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountainGen")
    FMountainGenSettings Settings;

private:
    void BuildChunkAndMesh();
};