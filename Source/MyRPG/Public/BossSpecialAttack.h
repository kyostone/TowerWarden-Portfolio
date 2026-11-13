// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGA_AreaAttack.h"
#include "BossSpecialAttack.generated.h"

/**
 * 
 */
UCLASS()
class MYRPG_API UBossSpecialAttack : public UMyGA_AreaAttack
{
	GENERATED_BODY()
	
private:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 보스 뒤쪽으로 순간이동 가능 위치 탐색
	bool FindTPLocation();

	// 특수 공격 범위의 중심점을 현재 타겟 플레이어로 변경
	virtual AActor* GetAreaCenterActor() const override;

	UFUNCTION()
	void OnTeleportDelayFinished();

	// 순간이동 될 최종 위치
	FVector TPLocation;
public:
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportDelay = 0.15f;

	// 순간이동 거리 탐색 범위
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TPDistance = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float MinTPDistance = 150.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TPDistanceStep = 75.f;
	
};
