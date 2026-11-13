// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAttackState.h"
#include "BaseEnemy.h"
#include "MyRPGCharacter.h"
#include "EnemyCombatStats.h"
void UMeleeAttackState::Enter(ABaseEnemy* Owner)
{

}

void UMeleeAttackState::Update(ABaseEnemy* Owner)
{
	if (!Owner || !Owner->GetCurrentTarget()) return;

	Owner->PrepareNextAttack();

	if (!Owner->IsReadyToAttack()) // 공격 가능 여부 체크
	{
		Owner->ChangeState(Owner->ChaseState);
		return;
	}

	Owner->PerformAttack();
}

void UMeleeAttackState::Exit(ABaseEnemy* Owner)
{

}