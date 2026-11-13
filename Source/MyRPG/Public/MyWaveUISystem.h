// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyRPGGameState.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MyWaveUISystem.generated.h"

/**
 * 
 */

class UMyGameHUDWidget;

UCLASS()
class MYRPG_API UMyWaveUISystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	void RegisterHUD(UMyGameHUDWidget* InHUD);
	void UnregisterHUD();

private:
	void BindGameState();
	void ApplyState(const FWaveUIState& State);
	void HandleWaveUIStateChanged(const FWaveUIState& State);
	void HandlePhaseChanged(const FWaveUIState& State);

	void StartCountdownDisplay();
	void StopCountdownDisplay();
	void RefreshCountdown();

	void ShowAnnouncement(const FText& Text, float Duration);
	void HideAnnouncement();

private:
	TWeakObjectPtr<UMyGameHUDWidget> HUDWidget;
	TWeakObjectPtr<AMyRPGGameState> BoundGameState;

	EWavePhase LastPhase = EWavePhase::Waiting;
	int32 LastWaveNumber = 0;

	FTimerHandle CountdownTimerHandle;
	FTimerHandle AnnouncementTimerHandle;
};