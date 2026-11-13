// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyState.h"
#include "ChaseState.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API UChaseState : public UEnemyState
{
	GENERATED_BODY()

public:
	virtual void Enter(ABaseEnemy* Owner) override;
	virtual void Update(ABaseEnemy* Owner) override;
	virtual void Exit(ABaseEnemy* Owner) override;
};
