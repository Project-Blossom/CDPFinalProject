#pragma once
#include "CoreMinimal.h"

// UE 리플렉션(UCLASS/USTRUCT) 없이, 순수 테이블 컨테이너
struct FMarchingCubesTables
{
    // 256 케이스별 edge mask (12 edges를 bit로 표현)
    static const int32 EdgeTable[256];

    // 256 케이스별 삼각형 구성
    // 각 행은 최대 5개의 triangle => 최대 15개의 vertex index, 끝은 -1
    static const int32 TriTable[256][16];
};
