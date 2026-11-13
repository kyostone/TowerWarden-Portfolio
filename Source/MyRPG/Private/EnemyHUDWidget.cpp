// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHUDWidget.h"
#include "Components/ProgressBar.h"

void UEnemyHUDWidget::UpdateHealthBar(float NewPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(NewPercent);
	}
}