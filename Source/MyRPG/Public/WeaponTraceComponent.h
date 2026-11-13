// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Damageable.h"
#include "Components/ActorComponent.h"
#include "WeaponTraceComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponTracePoint
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FName SocketName;

	FVector PreviousLocation;
};

class USoundBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYRPG_API UWeaponTraceComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UWeaponTraceComponent();
    void StartWeaponTrace(USkeletalMeshComponent* MeshComp);
    void UpdateWeaponTrace(USkeletalMeshComponent* MeshComp);
    void EndWeaponTrace();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Trace")
    TArray<FWeaponTracePoint> Locations;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Trace")
    float TraceRadius = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> WeaponHitSound;

    TSet<AActor*> HitActors;    

public:
	void SetDamageInfo(const FDamageInfo& DamageInfo)
	{
		CurrentDamageInfo = DamageInfo;
	}

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitSound(FVector HitLocation);

private:
	UPROPERTY(Transient)
	FDamageInfo CurrentDamageInfo;
};
