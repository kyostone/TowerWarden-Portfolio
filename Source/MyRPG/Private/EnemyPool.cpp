// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPool.h"
#include "MyWorldSubsystem.h"
#include "BaseEnemy.h"
void UEnemyPool::InitializePool()
{
	UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>();
	if (!Subsystem) return;

	const TMap<TSubclassOf<ABaseEnemy>, int32>& PoolingInfo = Subsystem->GetMonsterPoolCounts();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector PoolStorageLocation(0.f, 0.f, 10000.f);

	// Wave에 필요한 적들을 미리 생성
	for (const auto& EnemyInfo : PoolingInfo)
	{
		TSubclassOf<ABaseEnemy> EnemyClass = EnemyInfo.Key;

		for (int i = 0; i < EnemyInfo.Value; i++)
		{
			ABaseEnemy* NewEnemy = GetWorld()->SpawnActor<ABaseEnemy>(EnemyClass, PoolStorageLocation, FRotator::ZeroRotator, SpawnParams);

			if (!NewEnemy) continue;
			NewEnemy->SetReplicates(false);
			NewEnemy->Deactivate();
			EnemyClasses.FindOrAdd(EnemyClass).Add(NewEnemy);
		}
	}
}

// Pool에서 적 가져오기
ABaseEnemy* UEnemyPool::GetEnemy(TSubclassOf<ABaseEnemy> Enemy)
{
	if (!Enemy) return nullptr;

	TArray<ABaseEnemy*>* Pool = EnemyClasses.Find(Enemy);

	if (!Pool || Pool->Num() == 0) return nullptr;

	return Pool->Pop();
}

// Pool로 반환
void UEnemyPool::ReleaseEnemy(ABaseEnemy* Enemy)
{
	if (!Enemy) return;

	EnemyClasses.FindOrAdd(Enemy->GetClass()).Add(Enemy);
}