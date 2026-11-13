#include "MyGameplayAbility.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"

UMyGameplayAbility::UMyGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UMyGameplayAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("Ability Activated!"));
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 이동정지 로직
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character && Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
    }

    UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        TEXT("AttackPlayTask"),
        AttackMontage
    );
    if (PlayMontageTask)
    {
        PlayMontageTask->OnCompleted.AddDynamic(this, &UMyGameplayAbility::OnAbilityCompletedOrCancelled);
        PlayMontageTask->OnBlendOut.AddDynamic(this, &UMyGameplayAbility::OnAbilityCompletedOrCancelled);
        PlayMontageTask->OnInterrupted.AddDynamic(this, &UMyGameplayAbility::OnAbilityCompletedOrCancelled);
        PlayMontageTask->OnCancelled.AddDynamic(this, &UMyGameplayAbility::OnAbilityCompletedOrCancelled);
        PlayMontageTask->ReadyForActivation();
    }
    else
    {
        OnAbilityCompletedOrCancelled();
        return;
    }
    CheckHit();
}

void UMyGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // 이동 복구 로직
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character && Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // 다시 걸을 수 있게 복구
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMyGameplayAbility::OnAbilityCompletedOrCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMyGameplayAbility::CheckHit()
{
}