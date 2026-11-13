#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility_MeleeBase.h"
#include "MyGameplayAbility_MeleeAttack.generated.h"

UCLASS()
class MYRPG_API UMyGameplayAbility_MeleeAttack
	: public UMyGameplayAbility_MeleeBase
{
	GENERATED_BODY()

public:
	UMyGameplayAbility_MeleeAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;

	virtual bool ShouldEndOnMontageInterrupted() const override;
	virtual float GetAttackPower(UAbilitySystemComponent* ASC) const;
private:
	void PlayComboAttack(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	int32 ComboIndex = 0;

	float LastAttackTime = -100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float ComboResetTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackDelay = 0.3f;

	bool bComboTransition = false;
};