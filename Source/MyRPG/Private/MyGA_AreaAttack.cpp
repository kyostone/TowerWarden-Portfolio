// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGA_AreaAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "BaseEnemyAttributeSet.h"
#include "BossAttackArea.h"
#include "Damageable.h"
#include "Engine/OverlapResult.h"
#include "MyGameplayTag.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffectTypes.h"

UMyGA_AreaAttack::UMyGA_AreaAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 공격 판정은 서버에서, 시각/사운드 연출은 GameplayCue로 전달
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	ActivationOwnedTags.AddTag(MyGameplayTag::State_Attacking);
	ActivationOwnedTags.AddTag(MyGameplayTag::State_MovementLocked);
}

void UMyGA_AreaAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	// 이동 정지
	if (Character)
	{
		if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
		{
			AIController->StopMovement();
		}
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
	}
	AActor* CenterActor = GetAreaCenterActor(); // 공격 대상

	if (!CenterActor || !ChargeMontage || !AttackAreaClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector GroundLocation = FindGroundLocation(CenterActor); // 공격 대상의 바닥을 중심점으로 지정

	AttackArea = GetWorld()->SpawnActor<ABossAttackArea>(AttackAreaClass,GroundLocation,FRotator::ZeroRotator);

	if (AttackArea)
	{
		AttackArea->StartCharge(ChargeDuration, DamageRadius);
	}

	// ChargeDuration에 맞춰 몽타주 재생 속도 조절
	const float PlayRate = ChargeMontage->GetPlayLength() / ChargeDuration;

	UAbilityTask_PlayMontageAndWait* ChargeTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ChargeMontage,PlayRate);

	if (!ChargeTask)
	{
		OnMontageCancelled();
		return;
	}

	ChargeTask->OnBlendOut.AddDynamic(this, &UMyGA_AreaAttack::OnChargeCompleted); // 
	ChargeTask->OnCancelled.AddDynamic(this, &UMyGA_AreaAttack::OnMontageCancelled);
	//ChargeTask->OnInterrupted.AddDynamic(this,&UMyGA_AreaAttack::OnMontageCancelled);
	ChargeTask->ReadyForActivation();
}

void UMyGA_AreaAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (IsValid(AttackArea))
	{
		AttackArea->Destroy();
		AttackArea = nullptr;
	}

	Super::EndAbility(Handle,ActorInfo,ActivationInfo,bReplicateEndAbility,bWasCancelled);
}

void UMyGA_AreaAttack::OnChargeCompleted()
{
	if (!AttackMontage)
	{
		OnAttackCompleted();
		return;
	}

	UAbilityTask_PlayMontageAndWait* AttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AttackMontage);

	if (!AttackTask)
	{
		OnMontageCancelled();
		return;
	}

	AttackTask->OnCompleted.AddDynamic(this,&UMyGA_AreaAttack::OnAttackCompleted);
	AttackTask->OnCancelled.AddDynamic(this,&UMyGA_AreaAttack::OnMontageCancelled);
	AttackTask->OnInterrupted.AddDynamic(this,&UMyGA_AreaAttack::OnMontageCancelled);
	AttackTask->ReadyForActivation();
}

void UMyGA_AreaAttack::ApplyAreaDamage()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	AActor* SourceActor = GetAvatarActorFromActorInfo();

	if (!ASC || !SourceActor || !DamageEffectClass) return;

	const FVector Center = AttackArea ? AttackArea->GetActorLocation() : FindGroundLocation(GetAreaCenterActor());

	FDamageInfo DamageInfo;
	DamageInfo.Amount = DamageAmount;
	DamageInfo.Attacker = SourceActor;
	DamageInfo.DamageEffectClass = DamageEffectClass;

	TArray<FOverlapResult> Overlaps;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(DamageRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SourceActor);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	GetWorld()->OverlapMultiByObjectType(Overlaps,Center,FQuat::Identity,ObjectQueryParams,Sphere,QueryParams);

	TSet<AActor*> HitActors;

	// 액터 순회
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* Target = Result.GetActor();

		// 이미 리스트에 있는 액터는 건너뜀
		if (!Target || HitActors.Contains(Target)) continue;

		IDamageable* Damageable = Cast<IDamageable>(Target);
		if (!Damageable) continue;

		HitActors.Add(Target);
		Damageable->ApplyDamage(DamageInfo);
	}
}

AActor* UMyGA_AreaAttack::GetAreaCenterActor() const
{
	return GetAvatarActorFromActorInfo();
}

// 공격 중심 아래로 Trace하여 실제 지면 위치 계산
FVector UMyGA_AreaAttack::FindGroundLocation(AActor* CenterActor) const
{
	if (!CenterActor) return FVector::ZeroVector;

	const FVector Start = CenterActor->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	const FVector End = CenterActor->GetActorLocation() - FVector(0.f, 0.f, 1000.f);

	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetAvatarActorFromActorInfo());

	if (GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECC_Visibility,Params))
	{
		return HitResult.ImpactPoint + FVector(0.f, 0.f, 2.f);
	}
	return CenterActor->GetActorLocation();
}

void UMyGA_AreaAttack::OnAttackCompleted()
{
	SpawnImpactEffect(DamageRadius);

	ApplyAreaDamage();

	if (IsValid(AttackArea))
	{
		AttackArea->Destroy();
		AttackArea = nullptr;
	}

	// 리커버리 몽타주가 없으면 바로 공격 종료
	if (!RecoveryMontage)
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
		return;
	}

	// 공격 후 리커버리 몽타주 재생
	UAbilityTask_PlayMontageAndWait* RecoveryTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,RecoveryMontage);

	if (!RecoveryTask)
	{
		OnRecoveryCompleted();
		return;
	}

	RecoveryTask->OnCompleted.AddDynamic(this,&UMyGA_AreaAttack::OnRecoveryCompleted);
	RecoveryTask->OnCancelled.AddDynamic(this,&UMyGA_AreaAttack::OnMontageCancelled);
	RecoveryTask->OnInterrupted.AddDynamic(this,&UMyGA_AreaAttack::OnMontageCancelled);
	RecoveryTask->ReadyForActivation();
}

void UMyGA_AreaAttack::OnMontageCancelled()
{
	if (AttackArea)
	{
		AttackArea->Destroy();
		AttackArea = nullptr;
	}

	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}

void UMyGA_AreaAttack::OnRecoveryCompleted()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}

void UMyGA_AreaAttack::SpawnImpactEffect(float AttackRadius)
{
if (!IsValid(AttackArea)) return;
if (!ImpactCueTag.IsValid()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	FGameplayCueParameters CueParams; 
	CueParams.Location = AttackArea->GetActorLocation();
	CueParams.RawMagnitude = AttackRadius;

	ASC->ExecuteGameplayCue(ImpactCueTag,CueParams); // GameplayCue를 실행하여 공격 위치와 범위 전달
}

