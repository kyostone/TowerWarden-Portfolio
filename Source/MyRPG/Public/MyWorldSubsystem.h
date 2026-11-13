// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "MyWorldSubsystem.generated.h"

class AEnemySpawner;
class UWaveDataAsset;
class ABaseEnemy;
class UEnemyPool;
class APlayerTower;
class ABossEnemy;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTowerRegistered,APlayerTower*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossEnemyRegistered, ABossEnemy*);

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	Idle,           
	Preparing,      
	Spawning,       
	WaitingClear,   
	Finished        
};

UCLASS()
class MYRPG_API UMyWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
		
#pragma region Initialization
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
private:
	void InitializeTotalWaveCount();
#pragma endregion

#pragma region Wave
public:
	void StartWaveSequence();
	void StartWave(int32 WaveCount);
	void NotifyEnemyDeath();
private:
	void OnWaveCleared();
	void StartNextWave();
	void StartWaveCountdown();
#pragma endregion

#pragma region WaveData
public:
	void RegisterSpawner(AEnemySpawner* Spawner);
	void SpawnWave(int32 WaveCount);
	void InitializeWaveInfo(AEnemySpawner* Spawner);
	void InitializeEnemyPoolInfo();
private:
	// Wave State
	int32 CurrentWaveIndex = 0;
	int32 TotalWaveCount = 2;
	int32 AliveEnemyCount = 0;
	EWaveState CurrentState = EWaveState::Idle;

	// Wave Timer
	float WaveCountdownDuration = 3.f;
	float NextWaveDelay = 3.f;
	FTimerHandle StartWaveTimerHandle;
	FTimerHandle NextWaveTimerHandle;

	// Wave Data
	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TArray<TObjectPtr<UWaveDataAsset>> CurrentWaveData;
#pragma endregion

#pragma region Actors
public:
	void RegisterPlayerTower(APlayerTower* Tower);
	void RegisterBossEnemy(ABossEnemy* Boss);
	APlayerTower* GetPlayerTower() const;

	FOnPlayerTowerRegistered OnPlayerTowerRegistered;
	FOnBossEnemyRegistered OnBossEnemyRegistered;
private:
	UPROPERTY()
	TObjectPtr<APlayerTower> PlayerTower;

	UPROPERTY()
	TArray<AEnemySpawner*> RegisteredSpawners;

	UPROPERTY()
	TObjectPtr<UEnemyPool> EnemyPool;

	TMap<TSubclassOf<ABaseEnemy>, int32> MonsterPoolCounts;
#pragma endregion

#pragma region Getter
public:
	const TMap<TSubclassOf<ABaseEnemy>, int32>& GetMonsterPoolCounts() const { return MonsterPoolCounts; }
	int32 GetCurrentWave() const { return CurrentWaveIndex; }
	int32 GetTotalWave() const { return TotalWaveCount; }
	EWaveState GetCurrentState() const { return CurrentState; }
#pragma endregion

#pragma region GameControl
public:
	void RestartGame();
#pragma endregion
};
