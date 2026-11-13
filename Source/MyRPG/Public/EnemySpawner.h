// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "EnemySpawnInfo.h"
#include "EnemySpawner.generated.h"

class UEnemyPool;
class ABaseEnemy;

UCLASS()
class MYRPG_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnRadius;
	UPROPERTY(EditAnywhere, Category = "Spawn")
	float RespawnInterval = 7.f;
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGameplayTag SpawnerTag;

	UPROPERTY()
	TObjectPtr<UEnemyPool> EnemyPool;
	
	TMap<int32, TArray<FEnemySpawnInfo>> WaveSpawnInfo;

	//void InitializePool();                
	int32 TrySpawnEnemy(int32 WaveCount);
	void OnEnemyDeath(ABaseEnemy* Enemy);
	FVector GetRandomSpawnLocation() const;

};
