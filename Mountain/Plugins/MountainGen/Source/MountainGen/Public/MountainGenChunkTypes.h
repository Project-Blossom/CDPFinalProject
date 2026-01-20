#pragma once
#include "CoreMinimal.h"
#include "MountainGenChunkTypes.generated.h"

USTRUCT(BlueprintType)
struct FChunkCoord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 X = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Y = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Z = 0;

    FChunkCoord() = default;
    FChunkCoord(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

    bool operator==(const FChunkCoord& O) const { return X == O.X && Y == O.Y && Z == O.Z; }
};

FORCEINLINE uint32 GetTypeHash(const FChunkCoord& C)
{
    return HashCombine(HashCombine(::GetTypeHash(C.X), ::GetTypeHash(C.Y)), ::GetTypeHash(C.Z));
}