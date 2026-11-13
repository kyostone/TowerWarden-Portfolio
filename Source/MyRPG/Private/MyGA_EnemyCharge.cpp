#include "MyGA_EnemyCharge.h"
#include "BaseEnemyAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/RootMotionSource.h"
#include "BaseEnemy.h"
#include "AIController.h"

UMyGA_EnemyCharge::UMyGA_EnemyCharge()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UMyGA_EnemyCharge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ChargeAttack(Handle, ActorInfo, ActivationInfo);
}

void UMyGA_EnemyCharge::ChargeAttack(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(GetAvatarActorFromActorInfo());
	if (!Enemy)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 이동 정지
	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		AIController->StopMovement();
	}

	if (!ChargePrepareMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* PrepareTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargePrepareMontage);

	if (!PrepareTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PrepareTask->OnCompleted.AddDynamic(this, &UMyGA_EnemyCharge::StartCharge);
	PrepareTask->OnCancelled.AddDynamic(this, &UMyGA_EnemyCharge::OnChargeCancelled);
	PrepareTask->OnInterrupted.AddDynamic(this, &UMyGA_EnemyCharge::OnChargeCancelled);
	PrepareTask->ReadyForActivation();
}

void UMyGA_EnemyCharge::StartCharge()
{
	AActor* Attacker = GetAvatarActorFromActorInfo();
	if (!Attacker)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!SetupMeleeDamage())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!ChargeMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* ChargeMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeMontage);

	if (!ChargeMontageTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ChargeMontageTask->ReadyForActivation();

	// 돌진 방향과 속도 계산
	FVector ChargeDirection = Attacker->GetActorForwardVector();
	float ChargeSpeed = ChargeDistance / ChargeDuration;

	UAbilityTask_ApplyRootMotionConstantForce* ChargeTask =
		UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this,
			NAME_None,
			ChargeDirection, // 이동 방향
			ChargeSpeed,	 // 이동 속도
			ChargeDuration,  // 이동 시간
			false,
			nullptr,
			ERootMotionFinishVelocityMode::SetVelocity, 
			FVector::ZeroVector, // 이동 끝난 후 관성
			0.f,
			false);

	if (!ChargeTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ChargeTask->OnFinish.AddDynamic(this, &UMyGA_EnemyCharge::OnChargeFinished);
	ChargeTask->ReadyForActivation();
}

void UMyGA_EnemyCharge::OnChargeFinished()
{
	if (!RecoveryMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* RecoveryTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, RecoveryMontage);

	if (!RecoveryTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	RecoveryTask->OnCompleted.AddDynamic(this, &UMyGA_EnemyCharge::OnRecoveryFinished);
	RecoveryTask->OnCancelled.AddDynamic(this, &UMyGA_EnemyCharge::OnChargeCancelled);
	RecoveryTask->OnInterrupted.AddDynamic(this, &UMyGA_EnemyCharge::OnChargeCancelled);
	RecoveryTask->ReadyForActivation();
}

void UMyGA_EnemyCharge::OnRecoveryFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMyGA_EnemyCharge::OnChargeCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

float UMyGA_EnemyCharge::GetAttackPower(UAbilitySystemComponent* ASC) const
{
	return ASC->GetNumericAttribute(UBaseEnemyAttributeSet::GetAttackPowerAttribute());
}