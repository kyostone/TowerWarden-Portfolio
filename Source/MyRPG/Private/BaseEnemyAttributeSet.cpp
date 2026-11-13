// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "BaseEnemy.h"
#include "Net/UnrealNetwork.h"
#include "MyRPGCharacter.h"

void UBaseEnemyAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseEnemyAttributeSet, Health, OldHealth);
}

void UBaseEnemyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseEnemyAttributeSet, MaxHealth, OldMaxHealth);
}

void UBaseEnemyAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseEnemyAttributeSet, AttackPower, OldAttackPower);
}

void UBaseEnemyAttributeSet::OnRep_Speed(const FGameplayAttributeData& OldSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseEnemyAttributeSet, Speed, OldSpeed);
}

void UBaseEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBaseEnemyAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseEnemyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);		
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseEnemyAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseEnemyAttributeSet, Speed, COND_None, REPNOTIFY_Always);
}

void UBaseEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if (AMyRPGCharacter* Attacker = Cast<AMyRPGCharacter>(Data.EffectSpec.GetContext().GetInstigator()))
		{
			if (ABaseEnemy* Owner = Cast<ABaseEnemy>(GetOwningActor()))
			{
				Owner->ChangeTarget(Attacker);
			}
		}
		float NewHealth = FMath::Clamp(Health.GetCurrentValue(), 0.0f, MaxHealth.GetCurrentValue());
		SetHealth(NewHealth);
		if (NewHealth <= 0.0f)
		{
			OnDeath.Broadcast();
		}
	}
}

