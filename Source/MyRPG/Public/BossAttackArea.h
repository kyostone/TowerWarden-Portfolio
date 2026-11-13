// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossAttackArea.generated.h"

class UDecalComponent;
class USceneComponent;

UCLASS()
class MYRPG_API ABossAttackArea : public AActor
{
	GENERATED_BODY()

public:
	ABossAttackArea();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	void StartCharge(float Duration, float Radius);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Area")
	TObjectPtr<UDecalComponent> BaseDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Area")
	TObjectPtr<UDecalComponent> ChargeDecal;

private:
	float ChargeDuration = 1.f;
	float ElapsedTime = 0.f;

	bool bIsCharging = false;
};
