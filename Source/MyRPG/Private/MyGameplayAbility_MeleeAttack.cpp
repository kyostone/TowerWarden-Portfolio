#include "MyGameplayAbility_MeleeAttack.h"
#include "Engine/World.h"
#include "MyAttributeSet.h"
UMyGameplayAbility_MeleeAttack::UMyGameplayAbility_MeleeAttack()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMyGameplayAbility_MeleeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);
	PlayComboAttack(Handle,ActorInfo,ActivationInfo);
}

void UMyGameplayAbility_MeleeAttack::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle,ActorInfo,ActivationInfo);

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackDelay)
	{
		return;
	}

	bComboTransition = true;
	PlayComboAttack(Handle,ActorInfo,ActivationInfo);
	bComboTransition = false;
}

void UMyGameplayAbility_MeleeAttack::PlayComboAttack(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime > ComboResetTime)
	{
		ComboIndex = 0;
	}

	if (AttackMontages.IsEmpty() || !AttackMontages.IsValidIndex(ComboIndex))
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	UAnimMontage* SelectedMontage = AttackMontages[ComboIndex];

	if (!ExecuteMeleeAttack(SelectedMontage,Handle,ActorInfo,ActivationInfo))
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	ComboIndex++;

	if (ComboIndex >= AttackMontages.Num())
	{
		ComboIndex = 0;
	}

	LastAttackTime = CurrentTime;
}

// 콤보 전환 중 Montage Interrupt로 Ability가 종료되지 않도록 처리
bool UMyGameplayAbility_MeleeAttack::ShouldEndOnMontageInterrupted() const
{
	return !bComboTransition;
}

float UMyGameplayAbility_MeleeAttack::GetAttackPower(UAbilitySystemComponent* ASC) const
{
	return ASC->GetNumericAttribute(UMyAttributeSet::GetAttackPowerAttribute());
}