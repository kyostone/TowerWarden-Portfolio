// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveStartTrriger.generated.h"

class UStaticMeshComponent;

UCLASS()
class MYRPG_API AWaveStartTrriger : public AActor
{
	GENERATED_BODY()

public:
	AWaveStartTrriger();

	void Interact();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	bool bWaveStarted = false;
};
