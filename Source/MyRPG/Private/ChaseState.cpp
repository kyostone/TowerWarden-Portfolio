// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaseState.h"
#include "BaseEnemy.h"
#include "MyRPGCharacter.h"
#include "EnemyCombatStats.h"
#include "PlayerTower.h"

void UChaseState::Enter(ABaseEnemy* Owner)
{

}

void UChaseState::Update(ABaseEnemy* Owner)
{
	if (!Owner || !Owner->GetCurrentTarget()) return;

	// 감지 범위 내 플레이어 탐색 및 타겟 갱신
	Owner->TryPlayerDetected();

	// 플레이어가 추격 포기 거리를 벗어나면 타워로 타겟 변경
	if (Owner->ShouldGiveUpTarget())
	{
		Owner->SetTargetToTower();
		return;
	}

	if (!Owner->GetCurrentTarget()) return;

	// 다음 공격 준비
	Owner->PrepareNextAttack();

	if (Owner->IsReadyToAttack()) // 공격 가능 여부 체크
	{
		Owner->ChangeState(Owner->AttackState);
		return;
	}

	Owner->MoveToTarget();
}

void UChaseState::Exit(ABaseEnemy* Owner)
{

}