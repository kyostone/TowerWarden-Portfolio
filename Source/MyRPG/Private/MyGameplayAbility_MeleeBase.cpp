// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameplayAbility_MeleeBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "WeaponTraceComponent.h"
#include "MyAttributeSet.h"

UMyGameplayAbility_MeleeBase::UMyGameplayAbility_MeleeBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UMyGameplayAbility_MeleeBase::ExecuteMeleeAttack(
	UAnimMontage* Montage,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!Montage) return false;
	if (!SetupMeleeDamage()) return false;

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);

	if (!MontageTask) return false;

	MontageTask->OnCompleted.AddDynamic(this, &UMyGameplayAbility_MeleeBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UMyGameplayAbility_MeleeBase::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UMyGameplayAbility_MeleeBase::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	return true;
}

// WeaponTrace에 공격 정보 넘겨줌
bool UMyGameplayAbility_MeleeBase::SetupMeleeDamage()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AActor* Attacker = GetAvatarActorFromActorInfo();
	if (!ASC || !Attacker) return false;

	UWeaponTraceComponent* TraceComponent = Attacker->FindComponentByClass<UWeaponTraceComponent>();
	if (!TraceComponent) return false;

	FDamageInfo DamageInfo;
	DamageInfo.Amount = GetAttackPower(ASC);
	DamageInfo.Attacker = Attacker;
	DamageInfo.DamageEffectClass = DamageEffectClass;

	TraceComponent->SetDamageInfo(DamageInfo);

	return true;
}

void UMyGameplayAbility_MeleeBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}

void UMyGameplayAbility_MeleeBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}

void UMyGameplayAbility_MeleeBase::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack: Interrupted, ShouldEnd=%d"),
		ShouldEndOnMontageInterrupted());

	if (!ShouldEndOnMontageInterrupted())
	{
		return;
	}
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}

bool UMyGameplayAbility_MeleeBase::ShouldEndOnMontageInterrupted() const
{
	return true;
}

// 오버라이드 용도
float UMyGameplayAbility_MeleeBase::GetAttackPower(UAbilitySystemComponent* ASC) const
{
	return 0.f;
}