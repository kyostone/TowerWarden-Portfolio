#pragma once

#include "CoreMinimal.h"
#include "EnemyCombatStats.generated.h"

USTRUCT(BlueprintType)
struct FEnemyCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AggroRange = 300.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AggroAngle = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ChaseGiveUpRange = 700.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackRange = 100.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackAngle = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackTriggerAngle = 30.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DetectionRadius = 200.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float HealthRegen = 10.f;
};