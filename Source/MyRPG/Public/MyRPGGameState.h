// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyRPGGameState.generated.h"

class AMyRPGCharacter;

// UI 상태
UENUM(BlueprintType)
enum class EWavePhase : uint8
{
	Waiting,
	Countdown,
	Active,
	Cleared,
	GameClear
};

// 웨이브 및 보스 UI 상태를 서버에서 관리하고 클라이언트로 복제

USTRUCT(BlueprintType)
struct FWaveUIState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EWavePhase Phase = EWavePhase::Waiting;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentWave = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalWave = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 AliveEnemyCount = 0;

	UPROPERTY(BlueprintReadOnly)
	float PhaseEndServerTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bBossActive = false;

	UPROPERTY(BlueprintReadOnly)
	float BossHealthPercent = 0.f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWaveUIStateChanged,const FWaveUIState&);

UCLASS()
class MYRPG_API AMyRPGGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<AMyRPGCharacter*> RegisteredPlayers;

	void RegisterPlayer(AMyRPGCharacter* Player);
	void UnregisterPlayer(AMyRPGCharacter* Player);

	const FWaveUIState& GetWaveUIState() const
	{
		return WaveUIState;
	}

	FOnWaveUIStateChanged OnWaveUIStateChanged;

	// Server
	void BeginWaveCountdown(int32 NextWave,int32 InTotalWave,float EndServerTime);
	void BeginWave(int32 WaveNumber,int32 InTotalWave,int32 EnemyCount);

	void SetAliveEnemyCount(int32 NewCount);
	void SetWaveCleared(int32 WaveNumber);
	void SetGameClear();
	void SetBossUI(bool bActive,float HealthPercent);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(ReplicatedUsing = OnRep_WaveUIState)
	FWaveUIState WaveUIState;

	UFUNCTION()
	void OnRep_WaveUIState();

	void BroadcastWaveUIState();
};