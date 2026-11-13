// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGADodge.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"

UMyGADodge::UMyGADodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMyGADodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector DodgeDirection = GetDodgeDirection(); // 회피 방향 계산

	Character->LaunchCharacter(DodgeDirection * DodgeStrength, true, false); // 이동

	if (!DodgeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,DodgeMontage,1.0f,NAME_None,true,1.0f,0.0f,true);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UMyGADodge::OnDodgeFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UMyGADodge::OnDodgeFinished);
	MontageTask->OnCancelled.AddDynamic(this, &UMyGADodge::OnDodgeFinished);

	MontageTask->ReadyForActivation();
}

FVector UMyGADodge::GetDodgeDirection() const
{
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	if (!Character) return FVector::ZeroVector;

	FVector Direction = Character->GetLastMovementInputVector(); // 마지막 이동 입력 방향 기준으로 회피
	if (Direction.IsNearlyZero()) // 입력이 없으면 정면으로 회피
	{
		Direction = Character->GetActorForwardVector();
	}
	Direction.Z = 0.f;

	return Direction.GetSafeNormal();
}

void UMyGADodge::OnDodgeFinished()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}