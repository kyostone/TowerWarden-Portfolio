#include "WeaponTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Damageable.h"
#include "GameplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UWeaponTraceComponent::UWeaponTraceComponent()
{
    SetIsReplicatedByDefault(true); // 피격 사운드 Multicast RPC를 위해 Replication 활성화
}

void UWeaponTraceComponent::StartWeaponTrace(USkeletalMeshComponent* MeshComp)
{
    AActor* Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority()) return;
    if (!MeshComp) return;
    if (Locations.IsEmpty()) return;

    HitActors.Reset(); // 동일 대상 중복 피격 방지용 초기화

    for (FWeaponTracePoint& WeaponData : Locations)
    {
        WeaponData.PreviousLocation = MeshComp->GetSocketLocation(WeaponData.SocketName);
    }
}

void UWeaponTraceComponent::UpdateWeaponTrace(USkeletalMeshComponent* MeshComp)
{
    AActor* Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority()) return;
    if (!MeshComp) return;
    if (Locations.IsEmpty()) return;

    FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner); // 자기자신 제외

    for (FWeaponTracePoint& WeaponData : Locations)
    {
        FVector CurrentLocation = MeshComp->GetSocketLocation(WeaponData.SocketName);

        TArray<FHitResult> OutHits;

        // 이전 프레임 소켓 위치부터 현재 위치까지 Sweep하여 빠른 무기 이동 중 타격 누락 방지
        GetWorld()->SweepMultiByChannel(
            OutHits,
            WeaponData.PreviousLocation,
            CurrentLocation,
            FQuat::Identity,
            ECC_GameTraceChannel1,
            Sphere,
            Params
        );

        WeaponData.PreviousLocation = CurrentLocation;

        for (FHitResult& Hit : OutHits)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor) continue;
            if (HitActors.Contains(HitActor)) continue;

            if (!UGameplayFunctionLibrary::CanDamage(Owner, HitActor))
            {
                continue;
            }

            IDamageable* DamageableTarget = Cast<IDamageable>(HitActor);
            if (!DamageableTarget) continue;

            HitActors.Add(HitActor);

            DamageableTarget->ApplyDamage(CurrentDamageInfo);

            MulticastPlayHitSound(Hit.ImpactPoint);
        }
    }
}

void UWeaponTraceComponent::EndWeaponTrace()
{
}

void UWeaponTraceComponent::MulticastPlayHitSound_Implementation(FVector HitLocation)
{
    if (WeaponHitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this,WeaponHitSound,HitLocation);
    }
}