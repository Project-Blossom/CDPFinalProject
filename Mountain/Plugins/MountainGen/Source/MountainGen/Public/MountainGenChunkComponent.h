#pragma once
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "MountainGenChunkTypes.h"
#include "MountainGenChunkComponent.generated.h"

UCLASS(ClassGroup = (MountainGen), meta = (BlueprintSpawnableComponent))
class MOUNTAINGEN_API UMountainGenChunkComponent : public UProceduralMeshComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen")
    FChunkCoord Coord;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MountainGen")
    int32 Revision = 0;

    void Init(const FChunkCoord& InCoord, int32 InRevision)
    {
        Coord = InCoord;
        Revision = InRevision;
        bUseAsyncCooking = true;
    }
};