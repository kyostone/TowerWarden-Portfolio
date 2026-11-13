// Fill out your copyright notice in the Description page of Project Settings.


#include "IdleState.h"
#include "BaseEnemy.h"
#include "MyRPGCharacter.h"
#include "EnemyCombatStats.h"
#include "BaseEnemyAttributeSet.h"
#include "Components/WidgetComponent.h"

void UIdleState::Enter(ABaseEnemy* Owner)
{
	Owner->MoveToTarget();
}

void UIdleState::Update(ABaseEnemy* Owner)
{
	if (!Owner) return;

	if (!Owner->TargetPlayer && Owner->TryPlayerDetected())
	{
		Owner->SpawnLocation = Owner->GetActorLocation();
		Owner->ChangeState(Owner->ChaseState);		
	}		
	else if(Owner->WidgetComponent->IsVisible())
	{
		if (Owner->AttributeSet->GetMaxHealth() != Owner->AttributeSet->GetHealth())
		{
			float NewHealth = FMath::Clamp(Owner->AttributeSet->GetHealth() + (Owner->CombatStats.HealthRegen * 0.1), 0.0f, Owner->AttributeSet->GetMaxHealth());
			Owner->AttributeSet->SetHealth(NewHealth);
		}
	}
}

void UIdleState::Exit(ABaseEnemy* Owner)
{

}