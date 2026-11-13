// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGA_EnemyMelee.h"
#include "BaseEnemyAttributeSet.h"

UMyGA_EnemyMelee::UMyGA_EnemyMelee()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMyGA_EnemyMelee::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ExecuteMeleeAttack(AttackMontage, Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

float UMyGA_EnemyMelee::GetAttackPower(UAbilitySystemComponent* ASC) const
{
	return ASC->GetNumericAttribute(UBaseEnemyAttributeSet::GetAttackPowerAttribute());
}