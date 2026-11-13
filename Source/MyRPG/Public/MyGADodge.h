// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MyGADodge.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API UMyGADodge : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UMyGADodge();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	FVector GetDodgeDirection() const;

	UFUNCTION()
	void OnDodgeFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeStrength = 900.f;
};
