#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbDebugComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UClimbDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClimbDebugComponent();

protected:
	virtual void BeginPlay() override;

public:
	// BP에서 Q/E/클릭으로 호출할 함수들
	UFUNCTION(BlueprintCallable, Category = "ClimbDebug")
	void EnterDebugFP();

	UFUNCTION(BlueprintCallable, Category = "ClimbDebug")
	void ExitDebugFP();

	UFUNCTION(BlueprintCallable, Category = "ClimbDebug")
	void TraceAndPrintAngle();

	// ===== Auto-Find Settings =====
	// 네 BP(또는 부모 C++) 컴포넌트 이름이 CameraBoom / FollowCamera 라면 기본값 그대로 두면 됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbDebug|AutoFind")
	FName CameraBoomName = TEXT("CameraBoom");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbDebug|AutoFind")
	FName FollowCameraName = TEXT("FollowCamera");

	// ===== Optional Refs (직접 지정해도 됨. 지정 안 하면 BeginPlay에서 AutoFind) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ClimbDebug|Refs")
	USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ClimbDebug|Refs")
	UCameraComponent* FollowCamera = nullptr;

	// ===== Settings =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbDebug|Settings")
	float TraceDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbDebug|Settings")
	FVector FP_SocketOffset = FVector(0.f, 0.f, 60.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbDebug|Settings")
	bool bHideMeshInFP = true;

private:
	bool bDebugFP = false;

	// 복구용
	float SavedBoomLength = 0.f;
	FVector SavedSocketOffset = FVector::ZeroVector;

	void PrintScreen(const FString& Msg, const FColor& Color, float Time = 2.0f) const;
	bool EnsureRefs() const;

	// AutoFind helpers
	void AutoFindRefs();
	template<typename T>
	static T* FindComponentByName(AActor* Owner, FName TargetName);
};