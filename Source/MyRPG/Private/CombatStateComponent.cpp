// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatStateComponent.h"
#include "MyGameplayTag.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UCombatStateComponent::UCombatStateComponent()
{

}


// Called when the game starts
void UCombatStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UCombatStateComponent::Initialize(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	AbilitySystemComponent = ASC;

	ASC->RegisterGameplayTagEvent(MyGameplayTag::State_MovementLocked).AddUObject(this, &UCombatStateComponent::MovementLock);
}

void UCombatStateComponent::MovementLock(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		CharacterStop();
	}
}

void UCombatStateComponent::CharacterStop()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;
	Character->GetCharacterMovement()->StopMovementImmediately();
}

void UCombatStateComponent::ToggleStateTag(const FGameplayTag& Tag)
{
	if (!AbilitySystemComponent) return;

	if (AbilitySystemComponent->HasMatchingGameplayTag(Tag))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(Tag);
	}
	else
	{
		AbilitySystemComponent->AddLooseGameplayTag(Tag);
	}
}
