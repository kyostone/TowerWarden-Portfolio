#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnemyState.generated.h"

class ABaseEnemy;

UCLASS(Abstract)
class MYRPG_API UEnemyState : public UObject
{
	GENERATED_BODY()
public:
	virtual void Enter(ABaseEnemy* Owner);
	virtual void Update(ABaseEnemy* Owner);
	virtual void Exit(ABaseEnemy* Owner);
};
