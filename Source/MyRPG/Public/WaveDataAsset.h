// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseEnemy.h"
#include "WaveDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSpawnerWaveInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SpawnerTag;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TArray<FDataTableRowHandle> SpawnEnemyInfo;
};

UCLASS(BlueprintType)
class MYRPG_API UWaveDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSpawnerWaveInfo> SpawnerWaveInfo;

	int32 GetMaxSpawnCount(TSubclassOf<ABaseEnemy> MonsterClass, int32 WaveCount);
};
