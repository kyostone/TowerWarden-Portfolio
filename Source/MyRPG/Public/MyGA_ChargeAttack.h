// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility_MeleeBase.h"
#include "MyGA_ChargeAttack.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API UMyGA_ChargeAttack : public UMyGameplayAbility_MeleeBase
{
	GENERATED_BODY()
public:
	UMyGA_ChargeAttack();
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual float GetAttackPower(UAbilitySystemComponent* ASC) const override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> ChargeMontage;

	float ChargeStartTime = 0.f;
	float MaxChargeTime = 1.f;

	bool bIsCharging = false;
};
