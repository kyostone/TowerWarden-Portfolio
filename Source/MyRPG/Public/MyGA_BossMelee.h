#include "CoreMinimal.h"
#include "MyGameplayAbility_MeleeBase.h"
#include "MyGA_BossMelee.generated.h"

UCLASS()
class MYRPG_API UMyGA_BossMelee : public UMyGameplayAbility_MeleeBase
{
	GENERATED_BODY()

public:
	UMyGA_BossMelee();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual float GetAttackPower(UAbilitySystemComponent* ASC) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
};