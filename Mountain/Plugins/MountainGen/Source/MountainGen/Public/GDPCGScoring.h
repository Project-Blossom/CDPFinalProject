#pragma once

#include "CoreMinimal.h"
#include "GDPCGTypes.h"

// ============================================================
// Goal-Driven PCG Scoring
// 랜덤 후보 결과를 목표 프로필과 비교하여 점수화한다.
// 점수는 낮을수록 목표에 가깝다.
// ============================================================

namespace GDPCGScoring
{
    float ScoreMetricValue(float Value, const FGDPCGMetricTarget& Target, bool& bOutPassed);

    FGDPCGCandidateResult EvaluateCandidate(
        int32 Seed,
        const FGDPCGTargetProfile& Profile,
        const TArray<FGDPCGMetricValue>& ObservedMetrics);

    FString MakeCompactResultLine(const FGDPCGCandidateResult& Result);
}
