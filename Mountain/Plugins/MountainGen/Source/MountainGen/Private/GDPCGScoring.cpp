#include "GDPCGScoring.h"

namespace GDPCGScoring
{
    static const FGDPCGMetricValue* FindMetricByName(
        const TArray<FGDPCGMetricValue>& Metrics,
        const FName MetricName)
    {
        for (const FGDPCGMetricValue& M : Metrics)
        {
            if (M.MetricName == MetricName)
            {
                return &M;
            }
        }

        return nullptr;
    }

    float ScoreMetricValue(float Value, const FGDPCGMetricTarget& Target, bool& bOutPassed)
    {
        const float Weight = FMath::Max(0.f, Target.Weight);
        bOutPassed = false;

        switch (Target.GoalType)
        {
        case EGDPCGMetricGoalType::LowerIsBetter:
            bOutPassed = (Value <= Target.MaxValue);
            return FMath::Max(0.f, Value - Target.MaxValue) * Weight;

        case EGDPCGMetricGoalType::HigherIsBetter:
            bOutPassed = (Value >= Target.MinValue);
            return FMath::Max(0.f, Target.MinValue - Value) * Weight;

        case EGDPCGMetricGoalType::Range:
        default:
            bOutPassed = (Value >= Target.MinValue && Value <= Target.MaxValue);
            if (Value < Target.MinValue)
            {
                return (Target.MinValue - Value) * Weight;
            }
            if (Value > Target.MaxValue)
            {
                return (Value - Target.MaxValue) * Weight;
            }
            return 0.f;
        }
    }

    FGDPCGCandidateResult EvaluateCandidate(
        int32 Seed,
        const FGDPCGTargetProfile& Profile,
        const TArray<FGDPCGMetricValue>& ObservedMetrics)
    {
        FGDPCGCandidateResult Result;
        Result.Seed = FMath::Max(1, Seed);
        Result.FinalScore = 0.f;
        Result.bAllTargetsPassed = true;
        Result.Metrics.Reserve(Profile.Targets.Num());

        for (const FGDPCGMetricTarget& Target : Profile.Targets)
        {
            FGDPCGMetricValue Evaluated;
            Evaluated.MetricName = Target.MetricName;

            const FGDPCGMetricValue* Found = FindMetricByName(ObservedMetrics, Target.MetricName);
            if (!Found)
            {
                // Missing metric means this generator cannot prove the target.
                // Penalize heavily instead of silently passing.
                Evaluated.Value = 0.f;
                Evaluated.Score = FMath::Max(1.f, Target.Weight) * 1000.f;
                Evaluated.bPassed = false;
            }
            else
            {
                Evaluated.Value = Found->Value;
                Evaluated.Score = ScoreMetricValue(Evaluated.Value, Target, Evaluated.bPassed);
            }

            Result.FinalScore += Evaluated.Score;
            Result.bAllTargetsPassed = Result.bAllTargetsPassed && Evaluated.bPassed;
            Result.Metrics.Add(Evaluated);
        }

        return Result;
    }

    FString MakeCompactResultLine(const FGDPCGCandidateResult& Result)
    {
        FString Line = FString::Printf(TEXT("seed=%d | Score=%.3f | %s"),
            Result.Seed,
            Result.FinalScore,
            Result.bAllTargetsPassed ? TEXT("PASS") : TEXT("FAIL"));

        for (const FGDPCGMetricValue& M : Result.Metrics)
        {
            Line += FString::Printf(
                TEXT(" | %s=%.3f(%s, %.3f)"),
                *M.MetricName.ToString(),
                M.Value,
                M.bPassed ? TEXT("OK") : TEXT("NO"),
                M.Score);
        }

        return Line;
    }
}
