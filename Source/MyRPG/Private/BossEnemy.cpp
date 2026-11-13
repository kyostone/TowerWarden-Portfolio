// Fill out your copyright notice in the Description page of Project Settings.


#include "BossEnemy.h"
#include "AbilitySystemComponent.h"
#include "BaseEnemyAttributeSet.h"
#include "MyGamePlayTag.h"
#include "MyWorldSubsystem.h"
#include "Components/WidgetComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "MyRPGGameState.h"

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;
	if (!AbilitySystemComponent) return;

	for (const FBossAttackData& Attack : AttackPatterns)
	{
		if (!Attack.AbilityClass) continue;

		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Attack.AbilityClass));
	}
}

void ABossEnemy::Activate()
{
	Super::Activate();

	// 보스 UI 초기 설정
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
		WidgetComponent->SetHiddenInGame(true);
	}

	// 서버에서 Boss UI 초기 상태 설정 후 모든 클라이언트에 Boss 등록 전달
	if (!HasAuthority()) return;

	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->SetBossUI(true, 1.f);
	}

	MulticastRegisterBoss();
}

void ABossEnemy::Deactivate()
{
	if (!HasAuthority()) return;

	// Boss UI 비활성화
	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->SetBossUI(false, 0.f);
	}

	Super::Deactivate();
}

void ABossEnemy::MulticastRegisterBoss_Implementation()
{
	if (UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>())
	{
		Subsystem->RegisterBossEnemy(this);
	}
}

// Boss UI 갱신
void ABossEnemy::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	Super::OnHealthChanged(Data);

	if (!AttributeSet) return;

	const float MaxHealth = AttributeSet->GetMaxHealth();
	if (MaxHealth <= 0.f) return;

	const float HealthRatio = Data.NewValue / MaxHealth;

	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->SetBossUI(true, HealthRatio);
	}

	if (HealthRatio <= 0.4f)
	{
		ChangePhase(EBossPhase::Phase3);
	}
	else if (HealthRatio <= 0.8f)
	{
		ChangePhase(EBossPhase::Phase2);
	}
}

void ABossEnemy::ChangePhase(EBossPhase NewPhase)
{
	if (NewPhase <= CurrentPhase) return;
	CurrentPhase = NewPhase;
}

// 다음 공격 선택
void ABossEnemy::SelectNextAttack()
{
	if (!GetCurrentTarget())
	{
		CurrentAttackIndex = INDEX_NONE;
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float Distance = GetDistanceTo(GetCurrentTarget());

	TArray<int32> Candidates;

	for (int32 i = 0; i < AttackPatterns.Num(); ++i)
	{
		const FBossAttackData& Attack = AttackPatterns[i];

		if (!Attack.AbilityClass) continue;
		if (Attack.UnlockPhase > CurrentPhase) continue; // 해당 페이즈에 사용 가능한지 판단
		if (CurrentTime - Attack.LastUsedTime < Attack.Cooldown) continue; // 쿨타임 체크
		if (Distance < Attack.MinRange) continue; // 공격 가능 거리 체크

		Candidates.Add(i);
	}

	if (Candidates.IsEmpty())
	{
		CurrentAttackIndex = INDEX_NONE;
		return;
	}
	// 사용 가능한 공격 중 하나를 랜덤 선택
	CurrentAttackIndex = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

// 현재 선택된 공격이 유효한지 체크
void ABossEnemy::PrepareNextAttack()
{
	if (!GetCurrentTarget())
	{
		CurrentAttackIndex = INDEX_NONE;
		return;
	}

	if (AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(MyGameplayTag::State_Attacking))
	{
		return;
	}

	if (AttackPatterns.IsValidIndex(CurrentAttackIndex))
	{
		const FBossAttackData& Attack = AttackPatterns[CurrentAttackIndex];

		const float CurrentTime = GetWorld()->GetTimeSeconds();

		const float Distance = GetDistanceTo(GetCurrentTarget());

		const bool bInvalidAttack =
			!Attack.AbilityClass ||
			Attack.UnlockPhase > CurrentPhase || // 해당 페이즈에 사용 가능한지 판단
			CurrentTime - Attack.LastUsedTime < Attack.Cooldown || // 쿨타임 체크
			Distance < Attack.MinRange; // 사용 가능 거리 체크

		if (bInvalidAttack)
		{
			CurrentAttackIndex = INDEX_NONE;
		}
	}

	// 공격이 유효하지 않다면 다시 선택
	if (CurrentAttackIndex == INDEX_NONE)
	{
		SelectNextAttack();
	}
}

// 사용 할 공격의 공격 가능 범위 체크
bool ABossEnemy::IsInCurrentAttackRange() const
{
	if (!GetCurrentTarget()) return false;
	if (!AttackPatterns.IsValidIndex(CurrentAttackIndex)) return false;

	const FBossAttackData& Attack = AttackPatterns[CurrentAttackIndex];

	const float Distance = GetDistanceTo(GetCurrentTarget());

	return Distance >= Attack.MinRange && Distance <= Attack.MaxRange;
}

bool ABossEnemy::IsReadyToAttack() const
{
	if (!GetCurrentTarget()) return false;
	
	if (AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(	MyGameplayTag::State_Attacking))
	{
		return false;
	}
	
	// 선택된 공격 패턴이 없으면
	if (AttackPatterns.IsValidIndex(CurrentAttackIndex))
	{
		//기본 공격 조건으로
		return IsInCurrentAttackRange();
	}

	return Super::IsReadyToAttack();
}

void ABossEnemy::PerformAttack()
{
	if (!HasAuthority()) return;
	if (!AbilitySystemComponent) return;
	if (!GetCurrentTarget()) return;

	if (AbilitySystemComponent->HasMatchingGameplayTag(
		MyGameplayTag::State_Attacking))
	{
		return;
	}

	if (AttackPatterns.IsValidIndex(CurrentAttackIndex))
	{
		FBossAttackData& Attack = AttackPatterns[CurrentAttackIndex];

		if (!Attack.AbilityClass)
		{
			CurrentAttackIndex = INDEX_NONE;
			return;
		}

		if (AbilitySystemComponent->TryActivateAbilityByClass(Attack.AbilityClass))
		{
			const float CurrentTime = GetWorld()->GetTimeSeconds();

			Attack.LastUsedTime = CurrentTime;

			LastAttackTime = CurrentTime;

			CurrentAttackIndex = INDEX_NONE;
		}

		return;
	}
	
	Super::PerformAttack();
}

void ABossEnemy::MulticastStartTeleport_Implementation()
{
	if (TeleportEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),TeleportEffect,GetActorLocation());		
	}

	if (TeleportStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportStartSound, GetActorLocation());
	}

	SetActorHiddenInGame(true);
}

void ABossEnemy::MulticastFinishTeleport_Implementation(FVector NewLocation)
{
	SetActorHiddenInGame(false);

	if (TeleportEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TeleportEffect, NewLocation);
	}

	if (TeleportEndSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportEndSound, NewLocation);
	}
}

void ABossEnemy::MulticastPlayHitSound_Implementation()
{
	if (PlayHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), PlayHitSound, GetActorLocation());
	}
}