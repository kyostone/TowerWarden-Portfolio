#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemySpawnInfo.generated.h"

USTRUCT(BlueprintType)
struct FEnemySpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 WaveCount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class ABaseEnemy> EnemyClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Count = 1;
};