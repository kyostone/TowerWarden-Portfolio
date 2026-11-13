// Fill out your copyright notice in the Description page of Project Settings.


#include "BossSpecialAttack.h"
#include "BossEnemy.h"
#include "MyRPGCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

void UBossSpecialAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!FindTPLocation())
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	ABossEnemy* Owner = Cast<ABossEnemy>(GetAvatarActorFromActorInfo());
	if (!Owner)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	// 출발 위치 이펙트 재생 및 보스 숨김
	Owner->MulticastStartTeleport();

	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, TeleportDelay);
	WaitTask->OnFinish.AddDynamic(this,&UBossSpecialAttack::OnTeleportDelayFinished);
	WaitTask->ReadyForActivation();
}

bool UBossSpecialAttack::FindTPLocation()
{
	// 타겟 액터가 플레이어인지 검사
	if (!GetAreaCenterActor()) return false;

	ABossEnemy* Owner = Cast<ABossEnemy>(GetAvatarActorFromActorInfo());
	if (!Owner) return false;

	UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
	if (!Capsule) return false;

	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		Capsule->GetScaledCapsuleRadius(),
		Capsule->GetScaledCapsuleHalfHeight());

	FVector Forward = Owner->GetActorForwardVector();
	FVector Right = Owner->GetActorRightVector();

	// 보스 뒤쪽을 우선으로 순간이동 위치 탐색
	TArray<FVector> Directions =
	{
		-Forward,								  // 뒤
		(-Forward + Right * 0.5f).GetSafeNormal(), // 뒤-오른쪽
		(-Forward - Right * 0.5f).GetSafeNormal()  // 뒤-왼쪽
	};

	for (const FVector& Direction : Directions)
	{
		// 먼 거리부터 점차 가까운 위치까지 검사
		for (float Distance = TPDistance;
			Distance >= MinTPDistance;
			Distance -= TPDistanceStep)
		{
			FVector CandidateLocation = Owner->GetActorLocation() + Direction * Distance;

			// 해당 위치에 보스 캡슐이 들어갈 수 있는지 검사
			bool bBlocked = GetWorld()->OverlapBlockingTestByProfile(
				CandidateLocation,
				FQuat::Identity,
				Capsule->GetCollisionProfileName(),
				CapsuleShape);

			if (!bBlocked)
			{
				TPLocation = CandidateLocation;
				return true;
			}
		}
	}
	return false;
}

void UBossSpecialAttack::OnTeleportDelayFinished()
{
	ABossEnemy* Owner = Cast<ABossEnemy>(GetAvatarActorFromActorInfo());

	if (!Owner)
	{
		EndAbility(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(),true,true);
		return;
	}
	
	Owner->TeleportTo(TPLocation,Owner->GetActorRotation());

	// 도착 이펙트 재생 및 보스 다시 표시
	Owner->MulticastFinishTeleport(TPLocation);

	// 순간이동 성공 후 기존 범위 공격 실행
	Super::ActivateAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		nullptr);
}

AActor* UBossSpecialAttack::GetAreaCenterActor() const 
{
	ABossEnemy* Owner = Cast<ABossEnemy>(GetAvatarActorFromActorInfo());
	if (!Owner) return nullptr;

	// 현재 타겟 플레이어를 공격의 중심으로 설정
	return Cast<AMyRPGCharacter>(Owner->GetCurrentTarget());
}