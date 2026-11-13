// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "MyWorldSubsystem.h"
#include "BaseEnemy.h"
#include "EnemyPool.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	//PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>();

	if (Subsystem)
	{
		Subsystem->RegisterSpawner(this);
	}
}

// Called every frame
//void AEnemySpawner::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//}

// 현재 Wave에 설정된 적들을 Enemy Pool에서 가져와 생성
int32 AEnemySpawner::TrySpawnEnemy(int32 WaveCount)
{
	if (!HasAuthority()) return 0;

	// 현재 Wave에 해당되는 Spawn 정보 탐색
	TArray<FEnemySpawnInfo>* SpawnInfos = WaveSpawnInfo.Find(WaveCount);
	if (!SpawnInfos) return 0;

	int32 SpawnedCount = 0;

	for (FEnemySpawnInfo& SpawnEnemyInfo : *SpawnInfos)
	{
		for (int i = 0; i < SpawnEnemyInfo.Count; i++)
		{
			// Pool에 저장된 적 재사용
			ABaseEnemy* NewEnemy = EnemyPool->GetEnemy(SpawnEnemyInfo.EnemyClass);
			if (!NewEnemy) continue;

			NewEnemy->SetReplicates(true);
			NewEnemy->SetActorLocation(GetRandomSpawnLocation());
			NewEnemy->Activate();
			NewEnemy->MySpawner = this;
			SpawnedCount++;
		}
	}

	return SpawnedCount;
}

void AEnemySpawner::OnEnemyDeath(ABaseEnemy* Enemy)
{
	if (!HasAuthority()) return;
	if (!Enemy) return;
	EnemyPool->ReleaseEnemy(Enemy); // 적 사망시 Pool로 반환

	if (UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>())
	{
		Subsystem->NotifyEnemyDeath();
	}
}

// 스포너 내 랜덤 위치 스폰
FVector AEnemySpawner::GetRandomSpawnLocation() const
{
	FVector Origin = GetActorLocation();

	float RandomAngle = FMath::RandRange(0.f, 360.f);// 랜덤 각도

	float RandomDistance = FMath::RandRange(0.f, SpawnRadius);// 랜덤 거리

	float OffsetX = FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
	float OffsetY = FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;

	return Origin + FVector(OffsetX, OffsetY, 100.f);
}