// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Damageable.h"

void UGameplayFunctionLibrary::ApplyKnockback(AActor* TargetActor, FVector Direction, float Strength)
{
	if (!TargetActor) return;

	ACharacter* Character = Cast<ACharacter>(TargetActor);
	if (!Character) return;

	Character->LaunchCharacter(Direction.GetSafeNormal() * Strength, true, true);
}

bool UGameplayFunctionLibrary::CanDamage(AActor* Attacker, AActor* Target)
{
    if (!Attacker || !Target) return false;
    if (Attacker == Target) return false;

    IDamageable* AttackerDamageable = Cast<IDamageable>(Attacker);
    IDamageable* TargetDamageable = Cast<IDamageable>(Target);

    if (!AttackerDamageable) return false;
    if (!TargetDamageable) return false;

    if (AttackerDamageable->GetCombatTeam() == TargetDamageable->GetCombatTeam())
    {
        return false;
    }

    return true;
}