// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameplayTag.h"

namespace MyGameplayTag
{
	UE_DEFINE_GAMEPLAY_TAG(State_Stunned, "State.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_MovementLocked, "State.MovementLocked");
	UE_DEFINE_GAMEPLAY_TAG(State_Injured, "State.Injured");
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking"); 
	UE_DEFINE_GAMEPLAY_TAG(Event_AreaAttack_Hit, "Event.AreaAttack.Hit");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Boss_AreaImpact,"GameplayCue.Boss.AreaImpact");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Boss_SpecialExplosion,"GameplayCue.Boss.SpecialExplosion");
}
