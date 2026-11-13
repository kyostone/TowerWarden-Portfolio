// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Damageable.h"
#include "GameFramework/Actor.h"
#include "PlayerTower.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTowerHealthChanged, float);

UCLASS()
class MYRPG_API APlayerTower : public AActor, public IDamageable
{
	GENERATED_BODY()

public:
	APlayerTower();

	UFUNCTION(BlueprintCallable)
	void TakeTowerDamage(float DamageAmount);

	virtual void ApplyDamage(const FDamageInfo& DamageInfo) override;

	float GetDurabilityPercent() const;

	virtual ECombatTeam GetCombatTeam() const override
	{
		return ECombatTeam::Player;
	}

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameOverSignature OnGameOver;

	FOnTowerHealthChanged OnTowerHealthChanged;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Tower")
	float MaxDurability = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentDurability)
	float CurrentDurability = 0.f;

	UFUNCTION()
	void OnRep_CurrentDurability();

	void OnTowerDestroyed();
};