// Fill out your copyright notice in the Description page of Project Settings.


#include "MyRPGGameState.h"
#include "MyRPGCharacter.h"
#include "Net/UnrealNetwork.h"

void AMyRPGGameState::RegisterPlayer(AMyRPGCharacter* Player)
{
	if (!Player) return;

	RegisteredPlayers.AddUnique(Player);
}

void AMyRPGGameState::UnregisterPlayer(AMyRPGCharacter* Player)
{
	RegisteredPlayers.Remove(Player);
}

void AMyRPGGameState::BeginWaveCountdown(int32 NextWave,int32 InTotalWave,float EndServerTime)
{
	if (!HasAuthority()) return;

	WaveUIState.Phase = EWavePhase::Countdown;
	WaveUIState.CurrentWave = NextWave;
	WaveUIState.TotalWave = InTotalWave;
	WaveUIState.AliveEnemyCount = 0;
	WaveUIState.PhaseEndServerTime = EndServerTime;

	BroadcastWaveUIState();
}

void AMyRPGGameState::BeginWave(int32 WaveNumber,int32 InTotalWave,int32 EnemyCount)
{
	if (!HasAuthority()) return;

	WaveUIState.Phase =	EWavePhase::Active;
	WaveUIState.CurrentWave = WaveNumber;
	WaveUIState.TotalWave = InTotalWave;
	WaveUIState.AliveEnemyCount = FMath::Max(0, EnemyCount);
	WaveUIState.PhaseEndServerTime = 0.f;

	BroadcastWaveUIState();
}

void AMyRPGGameState::SetAliveEnemyCount(int32 NewCount)
{
	if (!HasAuthority()) return;

	WaveUIState.AliveEnemyCount = FMath::Max(0, NewCount);

	BroadcastWaveUIState();
}

void AMyRPGGameState::SetWaveCleared(int32 WaveNumber)
{
	if (!HasAuthority()) return;

	WaveUIState.Phase = EWavePhase::Cleared;
	WaveUIState.CurrentWave = WaveNumber;
	WaveUIState.AliveEnemyCount = 0;
	WaveUIState.PhaseEndServerTime = 0.f;

	BroadcastWaveUIState();
}

void AMyRPGGameState::SetGameClear()
{
	if (!HasAuthority()) return;

	WaveUIState.Phase = EWavePhase::GameClear;
	WaveUIState.AliveEnemyCount = 0;
	WaveUIState.PhaseEndServerTime = 0.f;

	BroadcastWaveUIState();
}

void AMyRPGGameState::SetBossUI(bool bActive,float HealthPercent)
{
	if (!HasAuthority()) return;

	WaveUIState.bBossActive = bActive;
	WaveUIState.BossHealthPercent = FMath::Clamp(HealthPercent,0.f,1.f);

	BroadcastWaveUIState();
}

// 클라이언트에서 복제된 웨이브 상태 전달
void AMyRPGGameState::OnRep_WaveUIState()
{
	OnWaveUIStateChanged.Broadcast(WaveUIState);
}

// 서버 UI 갱신 및 클라이언트 복제 요청
void AMyRPGGameState::BroadcastWaveUIState()
{
	OnWaveUIStateChanged.Broadcast(WaveUIState);

	ForceNetUpdate();
}

void AMyRPGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyRPGGameState,WaveUIState);
}