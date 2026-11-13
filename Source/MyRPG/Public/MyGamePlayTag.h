// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace MyGameplayTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stunned);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_MovementLocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Injured);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking); 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_AreaAttack_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Boss_AreaImpact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Boss_SpecialExplosion);
}
