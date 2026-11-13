// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "MyGA_AreaAttack.generated.h"

class ABossAttackArea;
class UAnimMontage;
class UGameplayEffect;
class UAbilitySystemComponent;
class UNiagaraSystem;

UCLASS(Abstract)
class MYRPG_API UMyGA_AreaAttack: public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMyGA_AreaAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION()
	void OnChargeCompleted();

	UFUNCTION()
	void OnAttackCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnRecoveryCompleted();

	void ApplyAreaDamage();

	FVector FindGroundLocation(AActor* CenterActor) const;

	virtual AActor* GetAreaCenterActor() const;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> ChargeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> RecoveryMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ABossAttackArea> AttackAreaClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float ChargeDuration = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DamageRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DamageAmount = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FGameplayTag ImpactCueTag;

	void SpawnImpactEffect(float AttackRadius);

	UPROPERTY()
	TObjectPtr<ABossAttackArea> AttackArea;
};