// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAttackArea.h"
#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"

ABossAttackArea::ABossAttackArea()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BaseDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("BaseDecal"));
	BaseDecal->SetupAttachment(SceneRoot);

	ChargeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("ChargeDecal"));
	ChargeDecal->SetupAttachment(SceneRoot);

	BaseDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	ChargeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	BaseDecal->SetSortOrder(0);
	ChargeDecal->SetSortOrder(1);

	ChargeDecal->SetRelativeScale3D(FVector(1.f, 0.01f, 0.01f));
}

void ABossAttackArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCharging) return;

	ElapsedTime += DeltaTime;

	// 충전 시간에 맞춰 중앙 Decal을 확장해 공격 타이밍 표시
	const float Alpha = FMath::Clamp(ElapsedTime / ChargeDuration, 0.f, 1.f);

	ChargeDecal->SetRelativeScale3D(FVector(1.f, Alpha, Alpha));

	if (Alpha >= 1.f)
	{
		bIsCharging = false;
		SetActorTickEnabled(false);
	}
}

// 공격 범위에 맞춰 Decal 크기를 설정하고 충전 표시 시작
void ABossAttackArea::StartCharge_Implementation(float Duration, float Radius)
{
	ChargeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	ElapsedTime = 0.f;
	bIsCharging = true;

	BaseDecal->DecalSize = FVector(BaseDecal->DecalSize.X, Radius, Radius);

	ChargeDecal->DecalSize = FVector(ChargeDecal->DecalSize.X, Radius, Radius);

	BaseDecal->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	ChargeDecal->SetRelativeScale3D(FVector(1.f, 0.01f, 0.01f));

	BaseDecal->MarkRenderStateDirty();
	ChargeDecal->MarkRenderStateDirty();

	SetActorTickEnabled(true);
}

