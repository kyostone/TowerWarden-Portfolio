// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "BossEnemy.generated.h"

class UGameplayAbility;
class USoundBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossHealthChanged,float);

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1,
	Phase2,
	Phase3
};

USTRUCT(BlueprintType)
struct FBossAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxRange = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Cooldown = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBossPhase UnlockPhase = EBossPhase::Phase1;

	float LastUsedTime = -9999.f;
};

/**
 * 
 */

class UNiagaraSystem;

UCLASS()
class MYRPG_API ABossEnemy : public ABaseEnemy
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) override;

public:
	virtual void PrepareNextAttack() override;
	virtual bool IsReadyToAttack() const override;
	virtual void PerformAttack() override;
	virtual void Activate() override;
	virtual void Deactivate() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRegisterBoss();

	FOnBossHealthChanged OnBossHealthChanged;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartTeleport();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFinishTeleport(FVector NewLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitSound();

protected:
	void SelectNextAttack();
	bool IsInCurrentAttackRange() const;

	void ChangePhase(EBossPhase NewPhase);

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TeleportEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> TeleportStartSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> TeleportEndSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> PlayHitSound;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	EBossPhase CurrentPhase = EBossPhase::Phase1;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<FBossAttackData> AttackPatterns;

	int32 CurrentAttackIndex = INDEX_NONE;
};
