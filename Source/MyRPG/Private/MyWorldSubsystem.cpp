// Fill out your copyright notice in the Description page of Project Settings.

#include "MyWorldSubsystem.h"
#include "EnemySpawner.h"
#include "WaveDataAsset.h"
#include "EnemySpawnInfo.h"
#include "Engine/DataTable.h"
#include "WaveWorldSettings.h"
#include "EnemyPool.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerTower.h"
#include "MyRPGGameState.h"
#include "MyGameInstance.h"

#pragma region Initialization

void UMyWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentState = EWaveState::Idle;
	CurrentWaveIndex = 0;
}

void UMyWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (InWorld.GetNetMode() == NM_Client) return;
	if (!InWorld.IsGameWorld()) return;

	int32 StartWave = 1;

	// 레벨 이동 시 전달된 시작 웨이브 옵션 확인
	if (const TCHAR* StartWaveOption = InWorld.URL.GetOption(TEXT("StartWave="), nullptr))
	{
		StartWave = FMath::Max(1, FCString::Atoi(StartWaveOption));
	}

	InitializeTotalWaveCount();

	StartWave = FMath::Clamp(StartWave, 1, FMath::Max(1, TotalWaveCount));
	CurrentWaveIndex = StartWave - 1;

	InitializeEnemyPoolInfo();

	EnemyPool = NewObject<UEnemyPool>(this);

	if (EnemyPool)
	{
		EnemyPool->InitializePool();
	}
}

// 총 웨이브 수 계산
void UMyWorldSubsystem::InitializeTotalWaveCount()
{
	TotalWaveCount = 0;

	AWaveWorldSettings* WorldSettings = Cast<AWaveWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings) return;

	for (UWaveDataAsset* WaveData : WorldSettings->SpawnEnemyInfo)
	{
		if (!WaveData) continue;

		for (const FSpawnerWaveInfo& SpawnerInfo : WaveData->SpawnerWaveInfo)
		{
			for (const FDataTableRowHandle& RowHandle : SpawnerInfo.SpawnEnemyInfo)
			{
				const FEnemySpawnInfo* SpawnInfo = RowHandle.GetRow<FEnemySpawnInfo>(TEXT("InitializeTotalWaveCount"));
				if (!SpawnInfo) continue;

				TotalWaveCount = FMath::Max(TotalWaveCount, SpawnInfo->WaveCount);
			}
		}
	}
}

#pragma endregion

#pragma region Wave

void UMyWorldSubsystem::StartWaveSequence()
{
	if (UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>())
	{
		GI->StartGameSession();
	}

	StartWaveCountdown();
}

void UMyWorldSubsystem::StartWave(int32 WaveCount)
{
	if (!GetWorld()->GetAuthGameMode()) return;

	CurrentState = EWaveState::Preparing;
	CurrentWaveIndex = WaveCount;
	AliveEnemyCount = 0;

	CurrentState = EWaveState::Spawning;

	// 시작 웨이브에 해당하는 적 소환
	for (AEnemySpawner* Spawner : RegisteredSpawners)
	{
		if (!Spawner) continue;

		AliveEnemyCount += Spawner->TrySpawnEnemy(WaveCount);
	}

	// GS에 웨이브 정보 전달
	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->BeginWave(CurrentWaveIndex, TotalWaveCount, AliveEnemyCount);
	}

	if (AliveEnemyCount <= 0)
	{
		OnWaveCleared();
		return;
	}

	CurrentState = EWaveState::WaitingClear;
}

void UMyWorldSubsystem::NotifyEnemyDeath()
{
	if (CurrentState != EWaveState::WaitingClear) return;

	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);

	// 생존중인 적 수 적용
	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->SetAliveEnemyCount(AliveEnemyCount);
	}

	// 적이 모두 죽을 시 클리어로 넘어감
	if (AliveEnemyCount == 0)
	{
		OnWaveCleared();
	}
}

void UMyWorldSubsystem::OnWaveCleared()
{
	CurrentState = EWaveState::Finished;

	AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>();

	// 웨이브 클리어, 스테이지 클리어 판단
	if (CurrentWaveIndex >= TotalWaveCount)
	{
		if (GS)
		{
			GS->SetGameClear();
		}

		UE_LOG(LogTemp, Warning, TEXT("All Waves Clear"));
		return;
	}

	if (GS)
	{
		GS->SetWaveCleared(CurrentWaveIndex);
	}

	GetWorld()->GetTimerManager().SetTimer(NextWaveTimerHandle, this, &UMyWorldSubsystem::StartWaveCountdown, NextWaveDelay, false);
}

void UMyWorldSubsystem::StartNextWave()
{
	StartWave(CurrentWaveIndex + 1);
}

// 웨이브 시작 카운트
void UMyWorldSubsystem::StartWaveCountdown()
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (World->GetNetMode() == NM_Client) return;
	if (CurrentWaveIndex >= TotalWaveCount) return;

	AMyRPGGameState* GS = World->GetGameState<AMyRPGGameState>();
	if (!GS) return;

	// 웨이브 정보 전달 및 타이머 서버, 클라이언트 동기화
	const int32 NextWaveNumber = CurrentWaveIndex + 1;
	const float EndServerTime = GS->GetServerWorldTimeSeconds() + WaveCountdownDuration;

	GS->BeginWaveCountdown(NextWaveNumber, TotalWaveCount, EndServerTime);

	World->GetTimerManager().SetTimer(StartWaveTimerHandle, this, &UMyWorldSubsystem::StartNextWave, WaveCountdownDuration, false);
}

#pragma endregion

#pragma region WaveData

void UMyWorldSubsystem::RegisterSpawner(AEnemySpawner* Spawner)
{
	if (!Spawner) return;

	if (RegisteredSpawners.Contains(Spawner))
	{
		return;
	}

	Spawner->EnemyPool = EnemyPool;
	RegisteredSpawners.Add(Spawner);

	InitializeWaveInfo(Spawner);
}

void UMyWorldSubsystem::SpawnWave(int32 WaveCount)
{
	if (!GetWorld()->GetAuthGameMode()) return;

	for (AEnemySpawner* Spawner : RegisteredSpawners)
	{
		if (!Spawner) continue;

		Spawner->TrySpawnEnemy(WaveCount);
	}
}

// 스포너에 웨이브 정보 할당
void UMyWorldSubsystem::InitializeWaveInfo(AEnemySpawner* Spawner)
{
	if (!Spawner) return;

	Spawner->WaveSpawnInfo.Empty();

	AWaveWorldSettings* WorldSettings = Cast<AWaveWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings) return;

	for (UWaveDataAsset* WaveData : WorldSettings->SpawnEnemyInfo)
	{
		if (!WaveData) continue;

		for (const FSpawnerWaveInfo& SpawnerInfo : WaveData->SpawnerWaveInfo)
		{
			// 스포너 태그가 같은 스포너에게 할당
			if (Spawner->SpawnerTag != SpawnerInfo.SpawnerTag) continue;

			for (const FDataTableRowHandle& RowHandle : SpawnerInfo.SpawnEnemyInfo)
			{
				const FEnemySpawnInfo* SpawnInfo = RowHandle.GetRow<FEnemySpawnInfo>(TEXT("WaveSpawn"));
				if (!SpawnInfo) continue;

				Spawner->WaveSpawnInfo.FindOrAdd(SpawnInfo->WaveCount).Add(*SpawnInfo);
			}
		}
	}
}

// 적 유닛마다 웨이브당 최대 마릿수 계산
void UMyWorldSubsystem::InitializeEnemyPoolInfo()
{
	MonsterPoolCounts.Empty();

	AWaveWorldSettings* WorldSettings = Cast<AWaveWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings) return;

	// 몬스터 최대 필요 수 계산
	for (int32 Wave = 1; Wave <= TotalWaveCount; ++Wave)
	{
		TMap<TSubclassOf<ABaseEnemy>, int32> WaveSpawnCounts;

		for (UWaveDataAsset* WaveData : WorldSettings->SpawnEnemyInfo)
		{
			if (!WaveData) continue;

			for (const FSpawnerWaveInfo& SpawnerInfo : WaveData->SpawnerWaveInfo)
			{
				for (const FDataTableRowHandle& RowHandle : SpawnerInfo.SpawnEnemyInfo)
				{
					const FEnemySpawnInfo* SpawnInfo = RowHandle.GetRow<FEnemySpawnInfo>(TEXT("InitializeMonsterPoolInfo"));

					if (!SpawnInfo || !SpawnInfo->EnemyClass) continue;
					if (SpawnInfo->WaveCount != Wave) continue;

					WaveSpawnCounts.FindOrAdd(SpawnInfo->EnemyClass) += SpawnInfo->Count;
				}
			}
		}

		for (const auto& Pair : WaveSpawnCounts)
		{
			int32& MaxCount = MonsterPoolCounts.FindOrAdd(Pair.Key);
			MaxCount = FMath::Max(MaxCount, Pair.Value);
		}
	}
}

#pragma endregion

#pragma region Actors

void UMyWorldSubsystem::RegisterPlayerTower(APlayerTower* Tower)
{
	if (!IsValid(Tower)) return;
	if (PlayerTower == Tower) return;

	PlayerTower = Tower;

	OnPlayerTowerRegistered.Broadcast(PlayerTower);
}

APlayerTower* UMyWorldSubsystem::GetPlayerTower() const
{
	return PlayerTower;
}

void UMyWorldSubsystem::RegisterBossEnemy(ABossEnemy* Boss)
{
	if (!Boss) return;

	OnBossEnemyRegistered.Broadcast(Boss);
}

#pragma endregion

#pragma region GameControl

// 현재 레벨 호출하여 재시작
void UMyWorldSubsystem::RestartGame()
{
	if (GetWorld()->GetNetMode() == NM_Client) return;
	if (!GetWorld()->IsGameWorld()) return;

	FString WorldName = GetWorld()->GetName();

	UGameplayStatics::OpenLevel(this, FName(WorldName), true);
}

#pragma endregion