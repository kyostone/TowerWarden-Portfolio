// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGA_ChargeAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "MyAttributeSet.h"
#include "MyGamePlayTag.h"

UMyGA_ChargeAttack::UMyGA_ChargeAttack()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(MyGameplayTag::State_MovementLocked);
}

void UMyGA_ChargeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ChargeStartTime = GetWorld()->GetTimeSeconds();

	if (!ChargeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ChargeMontage->bEnableAutoBlendOut = false;

	UAbilityTask_PlayMontageAndWait* ChargeTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeMontage);

	if (!ChargeTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	bIsCharging = true;
	ChargeTask->ReadyForActivation();
}

void UMyGA_ChargeAttack::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (!bIsCharging)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float HeldTime = CurrentTime - ChargeStartTime;

	if (HeldTime >= MaxChargeTime)
	{
		if (!ExecuteMeleeAttack(AttackMontage, Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
	bIsCharging = false;
}

float UMyGA_ChargeAttack::GetAttackPower(UAbilitySystemComponent* ASC) const
{
	return ASC->GetNumericAttribute(UMyAttributeSet::GetAttackPowerAttribute());
}