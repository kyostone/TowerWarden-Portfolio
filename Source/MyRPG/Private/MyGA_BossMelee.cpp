// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGA_BossMelee.h"
#include "BaseEnemyAttributeSet.h"

UMyGA_BossMelee::UMyGA_BossMelee()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMyGA_BossMelee::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ExecuteMeleeAttack(AttackMontage, Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

float UMyGA_BossMelee::GetAttackPower(UAbilitySystemComponent* ASC) const
{
	return ASC->GetNumericAttribute(UBaseEnemyAttributeSet::GetAttackPowerAttribute());
}
