#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility_MeleeBase.h"
#include "MyGA_EnemyCharge.generated.h"

UCLASS()
class MYRPG_API UMyGA_EnemyCharge : public UMyGameplayAbility_MeleeBase
{
	GENERATED_BODY()

public:
	UMyGA_EnemyCharge();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual float GetAttackPower(UAbilitySystemComponent* ASC) const override;

private:
	void ChargeAttack(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	UFUNCTION()
	void StartCharge();

	UFUNCTION()
	void OnChargeFinished();

	UFUNCTION()
	void OnChargeCancelled();

	UFUNCTION()
	void OnRecoveryFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	TObjectPtr<UAnimMontage> RecoveryMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	TObjectPtr<UAnimMontage> ChargePrepareMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	TObjectPtr<UAnimMontage> ChargeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float ChargeDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float ChargeDuration = 0.7f;
};