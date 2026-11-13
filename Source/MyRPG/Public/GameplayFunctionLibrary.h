// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API UGameplayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	public:
		UFUNCTION(BlueprintCallable, Category = "Combat")
		static void ApplyKnockback(AActor* TargetActor, FVector Direction, float Strength);

		UFUNCTION(BlueprintPure, Category = "Combat")
		static bool CanDamage(AActor* Attacker, AActor* Target);
};
