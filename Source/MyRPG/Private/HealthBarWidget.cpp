// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "TimerManager.h"

void UHealthBarWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	if (HealthBar)
	{
		CurrentPercent = HealthBar->GetPercent();
		TargetPercent = CurrentPercent;
	}
}

void UHealthBarWidget::UpdateHealthBar(float NewPercent)
{
	if (!HealthBar) return;

	TargetPercent = FMath::Clamp(NewPercent, 0.f, 1.f);

	// 최초 호출 시 애니메이션 없이 HP 값으로 동기화
	if (!bInitialized)
	{
		CurrentPercent = TargetPercent;
		HealthBar->SetPercent(CurrentPercent);
		bInitialized = true;
		return;
	}

	if (FMath::IsNearlyEqual(CurrentPercent, TargetPercent))
	{
		CurrentPercent = TargetPercent;
		HealthBar->SetPercent(CurrentPercent);
		return;
	}

	const int32 SafeStepCount = FMath::Max(1, StepCount);

	CurrentStep = 0;
	StepAmount = (TargetPercent - CurrentPercent) / SafeStepCount; // 추가/감소 값 계산

	if (GetWorld()->GetTimerManager().IsTimerActive(HealthUpdateTimerHandle))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(HealthUpdateTimerHandle,this,&UHealthBarWidget::UpdateHealthBarStep,UpdateInterval,true);
}

void UHealthBarWidget::UpdateHealthBarStep()
{
	if (!HealthBar)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthUpdateTimerHandle);
		return;
	}

	CurrentPercent += StepAmount;
	CurrentStep++;

	const int32 SafeStepCount = FMath::Max(1, StepCount);

	// 마지막 단계에서 정확한 목표값으로 보정 후 타이머 종료
	if (CurrentStep >= SafeStepCount)
	{
		CurrentPercent = TargetPercent; 
		GetWorld()->GetTimerManager().ClearTimer(HealthUpdateTimerHandle);
	}
	HealthBar->SetPercent(CurrentPercent);
}

void UHealthBarWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthUpdateTimerHandle);
	}
	UUserWidget::NativeDestruct();
}