// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

UCLASS()
class MYRPG_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 현재까지 해금된 스테이지
	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 HighestUnlockedStage = 1;

	//스테이지별 웨이브 데이터 저장
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<int32> ContinueWaves;
};
