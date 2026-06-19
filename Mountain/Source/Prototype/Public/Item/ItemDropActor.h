#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/ItemDefinition.h"
#include "ItemDropActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class UStaticMesh;
class USoundBase;
class USoundAttenuation;

UCLASS(Blueprintable)
class PROTOTYPE_API AItemDropActor : public AActor
{
    GENERATED_BODY()

public:
    AItemDropActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drop")
    TObjectPtr<USphereComponent> PickupSphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drop")
    TObjectPtr<UStaticMeshComponent> ItemMesh = nullptr;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
    int32 Count = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "10.0"))
    float PickupRadiusCm = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
    bool bAutoPickupOnOverlap = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Sound")
    TObjectPtr<USoundAttenuation> PickupSoundAttenuation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Sound")
    TObjectPtr<USoundBase> PickupSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Sound")
    TArray<FItemSoundVariant> PickupSoundVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Sound", meta = (ClampMin = "0.01", UIMin = "0.5", UIMax = "2.0"))
    float PickupSoundPitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Sound", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
    float PickupSoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Sound")
    EItemSoundPlaybackMode PickupSoundPlaybackMode = EItemSoundPlaybackMode::Play2D;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop|Visual")
    bool bRotateInWorld = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop|Visual", meta = (ClampMin = "0.0"))
    float RotationYawSpeedDegPerSec = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop|Visual")
    bool bBobInWorld = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop|Visual", meta = (ClampMin = "0.0"))
    float BobAmplitudeCm = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop|Visual", meta = (ClampMin = "0.0"))
    float BobSpeed = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Drop")
    void InitializeDrop(FName NewItemId, int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Drop")
    void InitializeDropWithMesh(FName NewItemId, int32 NewCount, UStaticMesh* NewVisualMesh);

    UFUNCTION(BlueprintCallable, Category = "Drop|Visual")
    void SetVisualMesh(UStaticMesh* NewVisualMesh);

    UFUNCTION(BlueprintCallable, Category = "Drop")
    bool TryPickup(AActor* Picker);

private:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    FVector InitialMeshRelativeLocation = FVector::ZeroVector;
    float VisualTime = 0.0f;
};