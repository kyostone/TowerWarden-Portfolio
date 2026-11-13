// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_WeaponTrace.h"
#include "WeaponTraceComponent.h"

void UAnimNotifyState_WeaponTrace::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    UWeaponTraceComponent* TraceComponent = Owner->FindComponentByClass<UWeaponTraceComponent>();

    if (!TraceComponent) return;
    
    TraceComponent->StartWeaponTrace(MeshComp);
}

void UAnimNotifyState_WeaponTrace::NotifyTick(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float FrameDeltaTime,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    UWeaponTraceComponent* TraceComponent = Owner->FindComponentByClass<UWeaponTraceComponent>();

    if (!TraceComponent) return;

    TraceComponent->UpdateWeaponTrace(MeshComp);
}

void UAnimNotifyState_WeaponTrace::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    UWeaponTraceComponent* TraceComponent = Owner->FindComponentByClass<UWeaponTraceComponent>();

    if (!TraceComponent) return;

    TraceComponent->EndWeaponTrace();
}