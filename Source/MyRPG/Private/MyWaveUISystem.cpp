// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWaveUISystem.h"
#include "MyGameHUDWidget.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Subsystem 종료 시 HUD 및 GameState 연결 정리
void UMyWaveUISystem::Deinitialize()
{
	UnregisterHUD();

	Super::Deinitialize();
}

// 기존 HUD 연결 정리 및 관련 값 초기화
void UMyWaveUISystem::UnregisterHUD()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		World->GetTimerManager().ClearTimer(AnnouncementTimerHandle);
	}

	if (BoundGameState.IsValid())
	{
		BoundGameState->OnWaveUIStateChanged.RemoveAll(this);
	}

	HUDWidget.Reset();
	BoundGameState.Reset();

	LastPhase = EWavePhase::Waiting;
	LastWaveNumber = 0;
}

// 사용할 HUD 연결
void UMyWaveUISystem::RegisterHUD(UMyGameHUDWidget* InHUD)
{
	if (!InHUD) return;

	UnregisterHUD();

	HUDWidget = InHUD;

	BindGameState();
}

// GameState 연결 후 웨이브 상태 변경 이벤트 바인딩
void UMyWaveUISystem::BindGameState()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AMyRPGGameState* GS = World->GetGameState<AMyRPGGameState>();
	if (!GS) return;

	BoundGameState = GS;

	GS->OnWaveUIStateChanged.RemoveAll(this);
	GS->OnWaveUIStateChanged.AddUObject(this, &UMyWaveUISystem::HandleWaveUIStateChanged);

	ApplyState(GS->GetWaveUIState());
}

void UMyWaveUISystem::HandleWaveUIStateChanged(const FWaveUIState& State)
{
	ApplyState(State);
}

// 전달받은 State 값 적용
void UMyWaveUISystem::ApplyState(const FWaveUIState& State)
{
	if (!HUDWidget.IsValid()) return;

	HUDWidget->SetWaveInfo(State.CurrentWave, State.TotalWave);
	HUDWidget->SetEnemyCount(State.AliveEnemyCount);
	HUDWidget->SetBossHealth(State.bBossActive, State.BossHealthPercent);

	const bool bWaveActive = State.Phase == EWavePhase::Active;

	HUDWidget->SetWaveVisible(bWaveActive);
	HUDWidget->SetEnemyCountVisible(bWaveActive);

	// 페이즈, 웨이브가 변경된 경우에만 일회성 UI 연출 실행
	const bool bPhaseChanged = State.Phase != LastPhase || State.CurrentWave != LastWaveNumber;

	if (bPhaseChanged)
	{
		HandlePhaseChanged(State);

		LastPhase = State.Phase;
		LastWaveNumber = State.CurrentWave;
	}
}

// 페이즈 단계별로 처리
void UMyWaveUISystem::HandlePhaseChanged(const FWaveUIState& State)
{
	StopCountdownDisplay();

	switch (State.Phase)
	{
	case EWavePhase::Waiting:
		HideAnnouncement();
		break;

	case EWavePhase::Countdown:
		StartCountdownDisplay();
		break;

	case EWavePhase::Active:
		ShowAnnouncement(FText::Format(FText::FromString(TEXT("WAVE {0} START")), FText::AsNumber(State.CurrentWave)), 1.5f);
		break;

	case EWavePhase::Cleared:
		ShowAnnouncement(FText::Format(FText::FromString(TEXT("WAVE {0} CLEAR")), FText::AsNumber(State.CurrentWave)), 1.5f);
		break;

	case EWavePhase::GameClear:
		ShowAnnouncement(FText::FromString(TEXT("GAME CLEAR")), 0.f);
		HUDWidget->ShowClearUI();
		break;
	}
}

void UMyWaveUISystem::StartCountdownDisplay()
{
	RefreshCountdown();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UMyWaveUISystem::RefreshCountdown, 0.1f, true);
	}
}

void UMyWaveUISystem::StopCountdownDisplay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}

// 카운트 다운
void UMyWaveUISystem::RefreshCountdown()
{
	if (!HUDWidget.IsValid() || !BoundGameState.IsValid()) return;

	const FWaveUIState& State = BoundGameState->GetWaveUIState();

	// 페이즈 확인
	if (State.Phase != EWavePhase::Countdown)
	{
		StopCountdownDisplay();
		return;
	}

	// 서버 기준 카운트 다운 받아와 화면 표시용 Int형으로 변환
	const float Remaining = State.PhaseEndServerTime - BoundGameState->GetServerWorldTimeSeconds();
	const int32 Seconds = FMath::CeilToInt(FMath::Max(0.f, Remaining));

	if (Seconds <= 0) return;

	// 카운트 다운 출력
	HUDWidget->SetAnnouncement(FText::AsNumber(Seconds), true);
}

// 화면 출력
void UMyWaveUISystem::ShowAnnouncement(const FText& Text, float Duration)
{
	if (!HUDWidget.IsValid()) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AnnouncementTimerHandle);
	}

	HUDWidget->SetAnnouncement(Text, true);

	if (Duration <= 0.f) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AnnouncementTimerHandle, this, &UMyWaveUISystem::HideAnnouncement, Duration, false);
	}
}

void UMyWaveUISystem::HideAnnouncement()
{
	if (!HUDWidget.IsValid()) return;

	HUDWidget->SetAnnouncement(FText::GetEmpty(), false);
}

