// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameHUDWidget.h"
#include "Components/TextBlock.h"
#include "HealthBarWidget.h"

void UMyGameHUDWidget::SetWaveInfo(int32 CurrentWave, int32 TotalWave)
{
	if (!WaveText) return;

	WaveText->SetText(FText::Format(FText::FromString(TEXT("WAVE {0} / {1}")), FText::AsNumber(CurrentWave), FText::AsNumber(TotalWave)));
}

void UMyGameHUDWidget::SetEnemyCount(int32 Count)
{
	if (!EnemyCountText) return;

	EnemyCountText->SetText(FText::Format(FText::FromString(TEXT("ENEMIES {0}")), FText::AsNumber(Count)));
}

void UMyGameHUDWidget::SetWaveVisible(bool bVisible)
{
	if (!WaveText) return;

	WaveText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UMyGameHUDWidget::SetEnemyCountVisible(bool bVisible)
{
	if (!EnemyCountText) return;

	EnemyCountText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UMyGameHUDWidget::SetAnnouncement(const FText& Text, bool bVisible)
{
	if (!AnnouncementText) return;

	AnnouncementText->SetText(Text);
	AnnouncementText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UMyGameHUDWidget::SetBossHealth(bool bVisible, float Percent)
{
	if (!BossHealthBar) return;

	BossHealthBar->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bVisible)
	{
		BossHealthBar->UpdateHealthBar(Percent);
	}
}

void UMyGameHUDWidget::ShowClearUI()
{
	if (IsValid(ClearWidget)) return;
	if (!ClearWidgetClass) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ClearWidget = CreateWidget<UUserWidget>(PC, ClearWidgetClass);
	if (!ClearWidget) return;

	ClearWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ClearWidget->TakeWidget());

	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
}

