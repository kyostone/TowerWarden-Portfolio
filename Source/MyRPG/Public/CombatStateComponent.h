// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CombatStateComponent.generated.h"

class UAbilitySystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYRPG_API UCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatStateComponent();
	void ToggleStateTag(const FGameplayTag& Tag);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void Initialize(UAbilitySystemComponent* ASC);
	void MovementLock(FGameplayTag Tag, int32 NewCount);
	void CharacterStop();


	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
