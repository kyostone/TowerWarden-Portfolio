#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Damageable.h"
#include "EnemyCombatStats.h"
#include "WeaponTraceComponent.h"
#include "BaseEnemy.generated.h"

class UBaseEnemyAttributeSet;
class UAbilitySystemComponent;
class UEnemyState;
class AMyRPGCharacter;
class APlayerTower;
class UAnimMontage;
class USoundBase;
UCLASS()
class MYRPG_API ABaseEnemy : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

#pragma region Engine Overrides
public:
	// Sets default values for this character's properties
	ABaseEnemy();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
#pragma endregion

#pragma region AI State Management
public:
	FTimerHandle StateUpdateTimerHandle;

	void ChangeState(UEnemyState* NewState);
	void UpdateCurrentState();

	UPROPERTY()
	TObjectPtr<UEnemyState> CurrentState;

	UPROPERTY(EditDefaultsOnly, Category = "AI States")
	TSubclassOf<UEnemyState> IdleStateClass;
	UPROPERTY(EditDefaultsOnly, Category = "AI States")
	TSubclassOf<UEnemyState> ChaseStateClass;
	UPROPERTY(EditDefaultsOnly, Category = "AI States")
	TSubclassOf<UEnemyState> AttackStateClass;
	UPROPERTY(EditDefaultsOnly, Category = "AI States")
	TSubclassOf<UEnemyState> ReturnStateClass;

	UPROPERTY()
	TObjectPtr<UEnemyState> IdleState;
	UPROPERTY()
	TObjectPtr<UEnemyState> ChaseState;
	UPROPERTY()
	TObjectPtr<UEnemyState> AttackState;
	UPROPERTY()
	TObjectPtr<UEnemyState> ReturnState;
#pragma endregion

#pragma region Movement
public:
	void MoveToTarget();
	void MoveToLocation(FVector Location);

	UPROPERTY()
	FVector SpawnLocation;
#pragma endregion

#pragma region Combat
public:
	virtual void PerformAttack();
	bool CooldownCheck() const;
	virtual void PrepareNextAttack();
	virtual bool IsReadyToAttack() const;

	bool IsInSector(AActor* Target, float Range, float Angle) const;
	bool IsInRadius(AActor* Target, float Range) const;
	bool TryPlayerDetected();

	virtual void Deactivate();
	virtual void Activate();

	virtual void ApplyDamage(const FDamageInfo& DamageInfo) override;

	virtual bool CanStartAttack() const;
	bool ShouldGiveUpTarget() const;

	virtual ECombatTeam GetCombatTeam() const override
	{
		return ECombatTeam::Enemy;
	}

	UFUNCTION()
	void HandleDeath();
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayAbility> BasicAttackAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> StunEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FEnemyCombatStats CombatStats;	

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DeathMontage;

	FTimerHandle DeathTimerHandle;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDeathMontage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitMontage();

	void FinishDeath();

	UPROPERTY(BlueprintReadOnly, Category = "Spawn")
	class AEnemySpawner* MySpawner;	
	UPROPERTY()
	float LastAttackTime = -1.f;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackCooldown = 1.5f;

	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
private:
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<UWeaponTraceComponent> WeaponTraceComponent;
#pragma endregion

#pragma region GAS Components
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UBaseEnemyAttributeSet> AttributeSet;
#pragma endregion

#pragma region Initial Stats
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initial Stats")
	float InitialHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initial Stats")
	float InitialAttackPower = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initial Stats")
	float InitialSpeed = 300.f;
#pragma endregion

#pragma region Target
public:
	UPROPERTY()
	TObjectPtr<AMyRPGCharacter> TargetPlayer;
	UPROPERTY()
	TObjectPtr<APlayerTower> TargetTower;
	UPROPERTY()
	TObjectPtr<AMyRPGCharacter> LastAttacker;

	void SetTargetToTower();
	void ChangeTarget(AActor* NewTarget);
private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
public:
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }
private:
		void OnPlayerTowerRegistered(APlayerTower* Tower);
#pragma endregion

#pragma region UI
private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UEnemyHUDWidget> EnemyHUDClass;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* WidgetComponent;
	UPROPERTY()
	class UEnemyHUDWidget* EnemyHUDWidget;
	
	void SetHUDVisible(bool bVisible);
#pragma endregion	

#pragma region Sound
private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> DeathSound;
public:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDeathSound();
#pragma endregion 
};