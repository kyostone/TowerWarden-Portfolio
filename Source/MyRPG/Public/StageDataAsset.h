// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StageDataAsset.generated.h"

/**
 * 
 */

class UWorld;

UCLASS(BlueprintType)
class MYRPG_API UStageDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage")
	int32 StageIndex = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage")
	TSoftObjectPtr<UWorld> StageMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage")
	int32 StartWave = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage")
	FPrimaryAssetId NextStageId;
};
