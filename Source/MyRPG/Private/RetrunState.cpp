// Fill out your copyright notice in the Description page of Project Settings.


#include "RetrunState.h"
#include "BaseEnemy.h"

void UReturnState::Enter(ABaseEnemy* Owner)
{
	Owner->MoveToLocation(Owner->SpawnLocation);
}
void UReturnState::Update(ABaseEnemy* Owner)
{
	float Distance = FVector::Distance(Owner->GetActorLocation(), Owner->SpawnLocation);

	UE_LOG(LogTemp, Warning, TEXT("%f"),Distance);
	if (Distance <= ReturnAcceptanceRadius)
	{
		Owner->TargetPlayer = nullptr;
		Owner->ChangeState(Owner->IdleState);
	}
}
void UReturnState::Exit(ABaseEnemy* Owner)
{

}