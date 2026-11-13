// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "WaveDataAsset.h"
#include "WaveWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API AWaveWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<TObjectPtr<UWaveDataAsset>> SpawnEnemyInfo;
};
