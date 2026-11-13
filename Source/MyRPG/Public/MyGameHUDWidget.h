// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyGameHUDWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UHealthBarWidget;
class UUserWidget;

UCLASS()
class MYRPG_API UMyGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowClearUI();
	void SetWaveInfo(int32 CurrentWave, int32 TotalWave);
	void SetEnemyCount(int32 Count);
	void SetWaveVisible(bool bVisible);
	void SetEnemyCountVisible(bool bVisible);
	void SetAnnouncement(const FText& Text, bool bVisible);
	void SetBossHealth(bool bVisible, float Percent);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ClearWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> ClearWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyCountText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AnnouncementText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHealthBarWidget> BossHealthBar;
};
