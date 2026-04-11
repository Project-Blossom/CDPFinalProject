#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoltAnchorActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class PROTOTYPE_API ABoltAnchorActor : public AActor
{
    GENERATED_BODY()

public:
    ABoltAnchorActor();

    UFUNCTION(BlueprintPure, Category = "Bolt")
    FVector GetRopeAttachWorldLocation() const;

    UFUNCTION(BlueprintPure, Category = "Bolt")
    FVector GetHandSnapWorldLocation() const;

    UFUNCTION(BlueprintPure, Category = "Bolt")
    bool CanAttachSafetyLine() const;

    UFUNCTION(BlueprintCallable, Category = "Bolt")
    void SetSafetyLineAttached(bool bAttached, AActor* InUser);

    UFUNCTION(BlueprintCallable, Category = "Bolt")
    bool ConsumeDurability(float Amount);

    UFUNCTION(BlueprintPure, Category = "Bolt")
    float GetCurrentDurability() const { return CurrentDurability; }

    UFUNCTION(BlueprintPure, Category = "Bolt")
    bool IsBroken() const { return CurrentDurability <= 0.0f; }

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt")
    TObjectPtr<UStaticMeshComponent> BoltMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt")
    TObjectPtr<USceneComponent> HandSnapPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt")
    TObjectPtr<USceneComponent> RopeAttachPoint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bolt|Durability", meta = (ClampMin = "1.0"))
    float MaxDurability = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt|Durability")
    float CurrentDurability = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt|State")
    bool bSafetyLineAttached = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bolt|State")
    TObjectPtr<AActor> AttachedUser = nullptr;
};