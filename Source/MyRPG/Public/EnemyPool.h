// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnemyPool.generated.h"

class ABaseEnemy;

UCLASS()
class MYRPG_API UEnemyPool : public UObject
{
	GENERATED_BODY()
private:
	TMap<TSubclassOf<ABaseEnemy>,TArray<ABaseEnemy*>> EnemyClasses;
public:
	void InitializePool();
	void ReleaseEnemy(ABaseEnemy* Enemy);
	ABaseEnemy* GetEnemy(TSubclassOf<ABaseEnemy> Enemy);
};
