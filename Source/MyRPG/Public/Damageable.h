// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY()
	float Amount = 0.f;

	UPROPERTY()
	TObjectPtr<AActor> Attacker = nullptr;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> StunEffectClass;
};

UENUM(BlueprintType)
enum class ECombatTeam : uint8
{
	Neutral,
	Player,
	Enemy
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class MYRPG_API IDamageable
{
	GENERATED_BODY()

public:
	virtual void ApplyDamage(const FDamageInfo& DamageInfo) = 0;
	virtual ECombatTeam GetCombatTeam() const = 0;
};
