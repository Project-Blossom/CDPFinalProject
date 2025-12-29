#include "ClimbDebugComponent.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

UClimbDebugComponent::UClimbDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UClimbDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	AutoFindRefs();

	if (CameraBoom && FollowCamera)
	{
		PrintScreen(TEXT("ClimbDebug: AutoFind OK"), FColor::Green, 2.0f);
	}
	else
	{
		PrintScreen(TEXT("ClimbDebug: AutoFind FAILED (check component names)"), FColor::Red, 4.0f);
	}
}

void UClimbDebugComponent::PrintScreen(const FString& Msg, const FColor& Color, float Time) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Msg);
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
}

bool UClimbDebugComponent::EnsureRefs() const
{
	if (!CameraBoom || !FollowCamera)
	{
		PrintScreen(TEXT("ClimbDebug: CameraBoom/FollowCamera refs are NOT set."), FColor::Red, 4.0f);
		return false;
	}
	return true;
}

template<typename T>
T* UClimbDebugComponent::FindComponentByName(AActor* Owner, FName TargetName)
{
	if (!Owner) return nullptr;

	TArray<T*> Comps;
	Owner->GetComponents<T>(Comps);
	for (T* C : Comps)
	{
		if (C && C->GetFName() == TargetName)
		{
			return C;
		}
	}
	return nullptr;
}

void UClimbDebugComponent::AutoFindRefs()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 1) 이미 지정되어 있으면 그대로 사용
	if (CameraBoom && FollowCamera) return;

	// 2) 타입으로 먼저 하나 잡아보기 (가장 흔한 케이스는 이걸로 끝남)
	if (!CameraBoom)
	{
		CameraBoom = Owner->FindComponentByClass<USpringArmComponent>();
	}
	if (!FollowCamera)
	{
		FollowCamera = Owner->FindComponentByClass<UCameraComponent>();
	}

	// 3) 여러 개가 있을 수도 있으니, 이름으로 정확히 재탐색
	//    (네 스샷에서 Native Component Name이 CameraBoom이므로 기본값 그대로면 맞음)
	if (!CameraBoom)
	{
		CameraBoom = FindComponentByName<USpringArmComponent>(Owner, CameraBoomName);
	}
	if (!FollowCamera)
	{
		FollowCamera = FindComponentByName<UCameraComponent>(Owner, FollowCameraName);
	}
}

void UClimbDebugComponent::EnterDebugFP()
{
	if (bDebugFP) return;

	// 혹시 BeginPlay 전에 호출되면 대비
	if (!CameraBoom || !FollowCamera)
	{
		AutoFindRefs();
	}
	if (!EnsureRefs()) return;

	bDebugFP = true;

	SavedBoomLength = CameraBoom->TargetArmLength;
	SavedSocketOffset = CameraBoom->SocketOffset;

	CameraBoom->TargetArmLength = 0.f;
	CameraBoom->SocketOffset = FP_SocketOffset;

	// 1인칭처럼 컨트롤 회전을 카메라가 직접 따라가게
	FollowCamera->bUsePawnControlRotation = true;

	// 1인칭에서 자기 몸 가리기(선택)
	if (bHideMeshInFP)
	{
		if (ACharacter* Ch = Cast<ACharacter>(GetOwner()))
		{
			if (USkeletalMeshComponent* Mesh = Ch->GetMesh())
			{
				Mesh->SetOwnerNoSee(true);
			}
		}
	}

	PrintScreen(TEXT("Debug FP: ON (Q)"), FColor::Cyan, 2.0f);
}

void UClimbDebugComponent::ExitDebugFP()
{
	if (!bDebugFP) return;

	if (!CameraBoom || !FollowCamera)
	{
		AutoFindRefs();
	}
	if (!EnsureRefs()) return;

	bDebugFP = false;

	CameraBoom->TargetArmLength = SavedBoomLength;
	CameraBoom->SocketOffset = SavedSocketOffset;

	FollowCamera->bUsePawnControlRotation = false;

	if (bHideMeshInFP)
	{
		if (ACharacter* Ch = Cast<ACharacter>(GetOwner()))
		{
			if (USkeletalMeshComponent* Mesh = Ch->GetMesh())
			{
				Mesh->SetOwnerNoSee(false);
			}
		}
	}

	PrintScreen(TEXT("Debug FP: OFF (E)"), FColor::Cyan, 2.0f);
}

void UClimbDebugComponent::TraceAndPrintAngle()
{
	if (!bDebugFP) return;

	if (!CameraBoom || !FollowCamera)
	{
		AutoFindRefs();
	}
	if (!EnsureRefs()) return;

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector Dir = FollowCamera->GetForwardVector();
	const FVector End = Start + Dir * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ClimbDebugTrace), true);
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);

	if (!bHit)
	{
		PrintScreen(TEXT("No Hit"), FColor::Red, 1.0f);
		return;
	}

	DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Yellow, false, 1.0f);

	const FVector N = Hit.ImpactNormal.GetSafeNormal();
	float Dot = FVector::DotProduct(N, FVector::UpVector);
	Dot = FMath::Clamp(Dot, -1.f, 1.f);

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

	const FString Msg = FString::Printf(TEXT("Angle: %.2f deg | Dist: %.1f"), AngleDeg, Hit.Distance);
	PrintScreen(Msg, FColor::Green, 2.0f);
}