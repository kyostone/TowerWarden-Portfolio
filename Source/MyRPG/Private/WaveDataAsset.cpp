// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveDataAsset.h"
#include "MyWorldSubsystem.h"
#include "WaveWorldSettings.h"
#include "EnemySpawnInfo.h"

int32 UWaveDataAsset::GetMaxSpawnCount(TSubclassOf<ABaseEnemy> MonsterClass, int32 WaveCount)
{
	int32 SpawnCount = 0;

	for (const FSpawnerWaveInfo& SpawnerInfo : SpawnerWaveInfo)
	{
		for (const FDataTableRowHandle& RowHandle : SpawnerInfo.SpawnEnemyInfo)
		{
			const FEnemySpawnInfo* SpawnInfo =
				RowHandle.GetRow<FEnemySpawnInfo>(TEXT("GetSpawnCount"));

			if (!SpawnInfo) continue;

			if (SpawnInfo->EnemyClass == MonsterClass &&
				SpawnInfo->WaveCount == WaveCount)
			{
				SpawnCount += SpawnInfo->Count;
			}
		}
	}
	return SpawnCount;
}