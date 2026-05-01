#pragma once

#include "CoreMinimal.h"
#include "GDPCGTypes.generated.h"

// ============================================================
// Goal-Driven PCG Core Types
// 목표 기반 제어형 절차적 생성 공통 타입
// ============================================================

UENUM(BlueprintType)
enum class EGDPCGMetricGoalType : uint8
{
    // Value must stay between MinValue and MaxValue.
    Range UMETA(DisplayName = "Range"),

    // Lower value is better. MaxValue is treated as the acceptable upper limit.
    LowerIsBetter UMETA(DisplayName = "Lower Is Better"),

    // Higher value is better. MinValue is treated as the acceptable lower limit.
    HigherIsBetter UMETA(DisplayName = "Higher Is Better"),
};

USTRUCT(BlueprintType)
struct FGDPCGMetricTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    FName MetricName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    EGDPCGMetricGoalType GoalType = EGDPCGMetricGoalType::Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    float MinValue = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    float MaxValue = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric", meta = (ClampMin = "0.0"))
    float Weight = 1.f;
};

USTRUCT(BlueprintType)
struct FGDPCGMetricValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    FName MetricName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    float Value = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    float Score = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Metric")
    bool bPassed = false;
};

USTRUCT(BlueprintType)
struct FGDPCGTargetProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Profile")
    FName ProfileName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Profile")
    TArray<FGDPCGMetricTarget> Targets;
};

USTRUCT(BlueprintType)
struct FGDPCGCandidateResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Result")
    int32 Seed = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Result")
    float FinalScore = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Result")
    bool bAllTargetsPassed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoalDrivenPCG|Result")
    TArray<FGDPCGMetricValue> Metrics;
};
