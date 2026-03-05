#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemDropActor.generated.h"

class USphereComponent;

UCLASS()
class PROTOTYPE_API AItemDropActor : public AActor
{
    GENERATED_BODY()

public:
    AItemDropActor();

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> Sphere = nullptr;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
    int32 Count = 1;

private:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};