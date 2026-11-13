// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTower.h"
#include "MyWorldSubsystem.h"
#include "Net/UnrealNetwork.h"

APlayerTower::APlayerTower()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APlayerTower::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentDurability = MaxDurability;
	}

	// WorldSubsystem에 타워 참조 등록
	if (UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>())
	{
		Subsystem->RegisterPlayerTower(this);
	}
}

void APlayerTower::TakeTowerDamage(float Damage)
{
	if (!HasAuthority()) return;
	if (CurrentDurability <= 0.f) return;

	CurrentDurability = FMath::Max(0.f, CurrentDurability - Damage);

	// Listen Server UI 갱신용
	OnTowerHealthChanged.Broadcast(GetDurabilityPercent());

	if (CurrentDurability <= 0.f)
	{
		OnTowerDestroyed();
	}
}

void APlayerTower::ApplyDamage(const FDamageInfo& DamageInfo)
{
	TakeTowerDamage(DamageInfo.Amount);
}

float APlayerTower::GetDurabilityPercent() const
{
	if (MaxDurability <= 0.f) return 0.f;

	return CurrentDurability / MaxDurability;
}

void APlayerTower::OnRep_CurrentDurability()
{	
	OnTowerHealthChanged.Broadcast(GetDurabilityPercent());
}

void APlayerTower::OnTowerDestroyed()
{
	if (!HasAuthority()) return;

	OnGameOver.Broadcast();
}

void APlayerTower::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerTower, CurrentDurability);
}