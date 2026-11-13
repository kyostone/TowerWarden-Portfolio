// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility_MeleeBase.h"
#include "MyGA_EnemyMelee.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API UMyGA_EnemyMelee : public UMyGameplayAbility_MeleeBase
{
	GENERATED_BODY()
public:
	UMyGA_EnemyMelee();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual float GetAttackPower(UAbilitySystemComponent* ASC) const;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
};
