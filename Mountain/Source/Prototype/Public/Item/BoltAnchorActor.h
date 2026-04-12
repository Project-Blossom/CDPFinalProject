#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoltAnchorActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS(BlueprintType, Blueprintable)
class PROTOTYPE_API ABoltAnchorActor : public AActor
{
    GENERATED_BODY()

public:
    ABoltAnchorActor();

    UFUNCTION(BlueprintPure, Category = "Anchor")
    FVector GetRopeAttachWorldLocation() const;

    UFUNCTION(BlueprintPure, Category = "Anchor")
    FVector GetHandSnapWorldLocation() const;

    UFUNCTION(BlueprintPure, Category = "Anchor")
    bool CanAttachSafetyLine() const;

    UFUNCTION(BlueprintCallable, Category = "Anchor")
    void SetSafetyLineAttached(bool bAttached, AActor* InUser);

    UFUNCTION(BlueprintCallable, Category = "Anchor")
    bool ConsumeDurability(float Amount);

    UFUNCTION(BlueprintPure, Category = "Anchor")
    float GetCurrentDurability() const;

    UFUNCTION(BlueprintPure, Category = "Anchor")
    bool IsBroken() const;

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor")
    TObjectPtr<UStaticMeshComponent> AnchorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor")
    TObjectPtr<USceneComponent> HandSnapPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor")
    TObjectPtr<USceneComponent> RopeAttachPoint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor|Durability", meta = (ClampMin = "1.0"))
    float MaxDurability = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor|Durability")
    float CurrentDurability = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor|State")
    bool bSafetyLineAttached = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor|State")
    TObjectPtr<AActor> AttachedUser = nullptr;
};