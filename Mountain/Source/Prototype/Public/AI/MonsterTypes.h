#pragma once

#include "CoreMinimal.h"
#include "MonsterTypes.generated.h"

/**
 * 몬스터 기본 상태
 */
UENUM(BlueprintType)
enum class EMonsterState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Pursue      UMETA(DisplayName = "Pursue"),
    Attack      UMETA(DisplayName = "Attack"),
    Stunned     UMETA(DisplayName = "Stunned"),
    Dead        UMETA(DisplayName = "Dead")
};

/**
 * FlyingAttacker 공격 상태
 */
UENUM(BlueprintType)
enum class EAttackerState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Pursuing    UMETA(DisplayName = "Pursuing"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Cooldown    UMETA(DisplayName = "Cooldown")
};

/**
 * WallCrawler 상태
 */
UENUM(BlueprintType)
enum class EWallCrawlerState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Pursuing    UMETA(DisplayName = "Pursuing"),
    Attached    UMETA(DisplayName = "Attached"),
    Stunned     UMETA(DisplayName = "Stunned")
};
