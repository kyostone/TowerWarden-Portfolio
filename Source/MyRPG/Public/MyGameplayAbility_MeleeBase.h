// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MyGameplayAbility_MeleeBase.generated.h"

class UAnimMontage;
class UGameplayEffect;
/**
 *
 */
UCLASS(Abstract)
class MYRPG_API UMyGameplayAbility_MeleeBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMyGameplayAbility_MeleeBase();

protected:
	bool ExecuteMeleeAttack(
		UAnimMontage* Montage,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	);

	bool SetupMeleeDamage();

	virtual float GetAttackPower(UAbilitySystemComponent* ASC) const;

	virtual bool ShouldEndOnMontageInterrupted() const;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageInterrupted();

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};