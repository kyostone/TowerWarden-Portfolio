// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveStartTrriger.h"
#include "Components/StaticMeshComponent.h"
#include "MyWorldSubsystem.h"

AWaveStartTrriger::AWaveStartTrriger()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AWaveStartTrriger::Interact()
{
	if (!HasAuthority()) return;
	if (bWaveStarted) return;

	UMyWorldSubsystem* WaveSystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>();
	if (!WaveSystem) return;

	WaveSystem->StartWaveSequence();

	//시작 이후 중복 시작 및 상호작용 방지
	SetActorEnableCollision(false);
	bWaveStarted = true;
}

