// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHUDWidget.generated.h"

class UProgressBar;

UCLASS()
class MYRPG_API UEnemyHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

public:
	UFUNCTION()
	void UpdateHealthBar(float NewPercent);
};
