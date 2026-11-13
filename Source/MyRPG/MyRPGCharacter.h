// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Damageable.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "MyRPGCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UMyAbilitySystemComponent;
class UMyAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UWeaponTraceComponent;
class UAnimMontage;
class UHealthBarWidget;
class UUserWidget;
class AMyRPGGameState;
class APlayerTower;
class ABossEnemy;
class USoundBase;
class AWaveStartTrriger;

struct FInputActionValue;
struct FOnAttributeChangeData;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class AMyRPGCharacter : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

#pragma region Engine Overrides

public:
	AMyRPGCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Input Action

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChargeAttackAct;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* GameMenuAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DodgingAction;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAttack();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoCharge();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoChargeAttack();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OpenGameMenu();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void CloseGameMenu();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void Interact();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoDodge();

#pragma endregion

#pragma region Camera

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma endregion

#pragma region GAS

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UMyAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UMyAttributeSet* AttributeSet;

public:
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	virtual void ApplyDamage(const FDamageInfo& DamageInfo) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> BasicAttackAbility;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> ChargeAttackAbility;

	// 기본적으로 보유하고 있는 Ability
	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// 추후 획득하는 Ability
	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> LearnedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> DodgeAbility;

#pragma endregion

#pragma region UI

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerHUDWidget;

	UPROPERTY()
	TObjectPtr<UHealthBarWidget> PlayerHealthBarWidget;

	UPROPERTY()
	TObjectPtr<UHealthBarWidget> TowerHealthBarWidget;

	UPROPERTY()
	TObjectPtr<UHealthBarWidget> BossHealthBarWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameOverUI;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameMenuWidget;

	void InitializeHUD();
	void OnPlayerTowerRegistered(APlayerTower* Tower);
	void OnTowerHealthChanged(float NewPercent);
	void OnBossRegistered(ABossEnemy* Boss);
	void OnBossHealthChanged(float NewPercent);

public:
	UFUNCTION()
	void ShowGameOverScreen();

#pragma endregion

#pragma region Combat

public:
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> StunEffectClass;

	virtual ECombatTeam GetCombatTeam() const override
	{
		return ECombatTeam::Player;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DeathMontage;

private:
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<UWeaponTraceComponent> WeaponTraceComponent;

	UFUNCTION(NetMulticast, Reliable)
	void PlayDeathMontage();

	UFUNCTION()
	void HandleDeath();

#pragma endregion

#pragma region Sound

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> HitReactionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> DeathSound;

public:
	UFUNCTION(Client, Unreliable)
	void ClientPlayHitSound();

	UFUNCTION(Client, Unreliable)
	void ClientPlayDeathSound();

#pragma endregion

#pragma region Interaction

private:
	AWaveStartTrriger* FindWaveStartTrigger() const;

	UFUNCTION(Server, Reliable)
	void ServerInteract(AWaveStartTrriger* Target);

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionDistance = 300.f;

#pragma endregion
};