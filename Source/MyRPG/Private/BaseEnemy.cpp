// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "BaseEnemyAttributeSet.h"
#include "EnemyState.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "MyRPGCharacter.h"
#include "EnemyHUDWidget.h"
#include "MyRPGGameState.h"
#include "Components/WidgetComponent.h"
#include "EnemySpawner.h"
#include "GameplayFunctionLibrary.h"
#include "MyWorldSubsystem.h"
#include "PlayerTower.h"
#include "WeaponTraceComponent.h"
#include "MyGamePlayTag.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

#pragma region Engine Overrides
// Sets default values
ABaseEnemy::ABaseEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttributeSet = CreateDefaultSubobject<UBaseEnemyAttributeSet>(TEXT("AttributeSet"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WeaponTraceComponent = CreateDefaultSubobject<UWeaponTraceComponent>(TEXT("WeaponTraceComponent"));
	WidgetComponent->SetupAttachment(GetMesh());
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	//값 초기화
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		AttributeSet->InitHealth(InitialHealth);
		AttributeSet->InitMaxHealth(InitialHealth);
		AttributeSet->InitSpeed(InitialSpeed);
		AttributeSet->InitAttackPower(InitialAttackPower);
		AttributeSet->OnDeath.AddDynamic(this, &ABaseEnemy::HandleDeath);
	}

	if (IdleStateClass && ChaseStateClass && AttackStateClass && ReturnStateClass)
	{
		// AI 상태 객체 생성
		IdleState = NewObject<UEnemyState>(this, IdleStateClass);
		ChaseState = NewObject<UEnemyState>(this, ChaseStateClass);
		AttackState = NewObject<UEnemyState>(this, AttackStateClass);
		ReturnState = NewObject<UEnemyState>(this, ReturnStateClass);

		// 서버에서 초기 상태 설정 및 상태 업데이트 시작
		if (HasAuthority())
		{
			ChangeState(ChaseState);

			GetWorldTimerManager().SetTimer(StateUpdateTimerHandle, this, &ABaseEnemy::UpdateCurrentState, 0.1f, true);

			if (UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>())
			{
				Subsystem->OnPlayerTowerRegistered.AddUObject(this, &ABaseEnemy::OnPlayerTowerRegistered);

				if (APlayerTower* Tower = Subsystem->GetPlayerTower())
				{
					OnPlayerTowerRegistered(Tower);
				}
			}

			if (BasicAttackAbility)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BasicAttackAbility));
			}
		}
	}

	// 적 HP바 출력
	if (WidgetComponent)
	{
		WidgetComponent->SetWidgetClass(EnemyHUDClass);
		EnemyHUDWidget = Cast<UEnemyHUDWidget>(WidgetComponent->GetUserWidgetObject());

		if (EnemyHUDWidget && AbilitySystemComponent)
		{		
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseEnemyAttributeSet::GetHealthAttribute()).AddUObject(this, &ABaseEnemy::OnHealthChanged);
			
			SetHUDVisible(false);
		}
	}
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WidgetComponent && WidgetComponent->IsVisible())
	{
		APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
		if (CameraManager)
		{
			FVector ToCamera = CameraManager->GetCameraLocation() - WidgetComponent->GetComponentLocation();
			WidgetComponent->SetWorldRotation(ToCamera.Rotation());
		}
	}
}

UAbilitySystemComponent* ABaseEnemy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
#pragma endregion

#pragma region AI State Management
void ABaseEnemy::ChangeState(UEnemyState* NewState)
{
	if (CurrentState)
	{
		CurrentState->Exit(this);
	}
	CurrentState = NewState;
	if (CurrentState)
	{
		CurrentState->Enter(this);
	}
}

void ABaseEnemy::UpdateCurrentState()
{
	if (!HasAuthority()) return;
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned"))))
	{
		return;
	}
	if (CurrentState)
	{
		CurrentState->Update(this);
	}
}
#pragma endregion

#pragma region Movement
void ABaseEnemy::MoveToLocation(FVector Location)
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		AIController->MoveToLocation(Location);
	}
}

void ABaseEnemy::MoveToTarget()
{
	if (AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(MyGameplayTag::State_MovementLocked))
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController || !CurrentTarget) return;

	AIController->MoveToActor(CurrentTarget);
}
#pragma endregion

#pragma region Combat
void ABaseEnemy::PerformAttack()
{
	if (!HasAuthority()) return;
	if (!CurrentTarget) return;
	if (!CooldownCheck()) return;
	if (!AbilitySystemComponent || !BasicAttackAbility) return;

	if (AbilitySystemComponent->TryActivateAbilityByClass(BasicAttackAbility))
	{
		LastAttackTime = GetWorld()->GetTimeSeconds();
	}	
}

bool ABaseEnemy::CooldownCheck() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return false;
	}
	return true;
}

void ABaseEnemy::PrepareNextAttack()
{

}

// 일반 적은 기본 공격 조건을 사용하며, 보스는 공격 패턴에 맞게 오버라이드
bool ABaseEnemy::IsReadyToAttack() const
{
	return CanStartAttack();
}

bool ABaseEnemy::IsInSector(AActor* Target, float Range, float Angle) const
{
	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());
	FVector OwnerForward = GetActorForwardVector();

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	FVector DirToTarget = ToTarget.GetSafeNormal();

	float DotResult = FVector::DotProduct(DirToTarget, OwnerForward);
	float AttackTriggerRangeCos = FMath::Cos(FMath::DegreesToRadians(Angle));

	if (Range >= Distance && DotResult >= AttackTriggerRangeCos)
	{
		return true;
	}
	return false;
}

bool ABaseEnemy::IsInRadius(AActor* Target, float Range) const
{
	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());
	if (Distance <= Range)
	{
		return true;
	}
	return false;
}

// 플레이어가 감지 범위 내로 들어왔는지 판단
bool ABaseEnemy::TryPlayerDetected()
{
	AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>();
	if (!GS) return false;

	if (AMyRPGCharacter* CurrentPlayer = Cast<AMyRPGCharacter>(CurrentTarget))
	{
		if (IsInSector(CurrentPlayer, CombatStats.AggroRange, CombatStats.AggroAngle) ||
			IsInRadius(CurrentPlayer, CombatStats.DetectionRadius))
		{
			return true;
		}
	}

	AMyRPGCharacter* BestPlayer = nullptr;
	float BestDistance = MAX_FLT;

	// 모든 플레이어 순회
	for (AMyRPGCharacter* Player : GS->RegisteredPlayers)
	{
		if (!Player) continue;

		if (IsInSector(Player, CombatStats.AggroRange, CombatStats.AggroAngle) ||
			IsInRadius(Player, CombatStats.DetectionRadius))
		{
			float Distance = FVector::Distance(GetActorLocation(), Player->GetActorLocation());
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestPlayer = Player;
			}
		}
	}

	// 감지 범위 내 플레이어를 탐색하고 가장 가까운 플레이어로 타겟 변경
	if (BestPlayer)
	{
		ChangeTarget(BestPlayer);
		return true;
	}

	return false;
}

void ABaseEnemy::HandleDeath()
{
	if (!HasAuthority()) return;

	TargetPlayer = nullptr;
	CurrentTarget = nullptr;
	CurrentState = nullptr;

	SetActorEnableCollision(false);

	GetWorldTimerManager().ClearTimer(StateUpdateTimerHandle);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
		AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	}

	// 스포너에 전달
	if (MySpawner)
	{
		MySpawner->OnEnemyDeath(this);
	}
	
	// Death몽타주 없을 시 바로 비활성화
	if (!DeathMontage)
	{
		Deactivate();
		return;
	}

	MulticastPlayDeathMontage();
	MulticastPlayDeathSound();

	GetWorldTimerManager().SetTimer(DeathTimerHandle,this,&ABaseEnemy::FinishDeath,DeathMontage->GetPlayLength(),false);
}

// Death몽타주 종료 시 실행
void ABaseEnemy::FinishDeath()
{
	if (!HasAuthority()) return;

	Deactivate();
}

// 풀링 시 비활성화 할때 사용
void ABaseEnemy::Deactivate()
{
	GetWorldTimerManager().ClearTimer(StateUpdateTimerHandle); //타이머(업데이트) 종료
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

// 풀링 시 활성화 할때 사용
void ABaseEnemy::Activate()
{
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	GetWorldTimerManager().ClearTimer(DeathTimerHandle);

	if (AttributeSet && WidgetComponent)
	{
		AttributeSet->InitHealth(InitialHealth);
		AttributeSet->InitMaxHealth(InitialHealth);
		SetHUDVisible(false);
	}

	if (HasAuthority())
	{
		if (!CurrentTarget)
		{
			SetTargetToTower();
		}
		ChangeState(ChaseState);
		TargetPlayer = nullptr;
		GetWorldTimerManager().SetTimer(StateUpdateTimerHandle, this, &ABaseEnemy::UpdateCurrentState, 0.1f, true);
	}
}

void ABaseEnemy::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (!EnemyHUDWidget) return;

	float NewHealth = Data.NewValue;
	float MaxHealth = AttributeSet->GetMaxHealth();

	if (MaxHealth > 0.f)
	{
		if (NewHealth >= MaxHealth)
		{
			SetHUDVisible(false);
		}
		else
		{
			SetHUDVisible(true);
			EnemyHUDWidget->UpdateHealthBar(NewHealth / MaxHealth);
		}
	}
}

// 서버에서 유효한 공격인지 검사한 뒤 GameplayEffect로 데미지 적용
void ABaseEnemy::ApplyDamage(const FDamageInfo& DamageInfo)
{
	if (!HasAuthority()) return;
	if (!AbilitySystemComponent || !AttributeSet) return;
	if (!DamageInfo.Attacker) return;
	if (!DamageInfo.DamageEffectClass) return;
	if (!UGameplayFunctionLibrary::CanDamage(DamageInfo.Attacker, this)) return;
	
	IAbilitySystemInterface* AttackerASI = Cast<IAbilitySystemInterface>(DamageInfo.Attacker);
	if (!AttackerASI) return;

	UAbilitySystemComponent* SourceASC = AttackerASI->GetAbilitySystemComponent();
	if (!SourceASC) return;

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(DamageInfo.Attacker);
	FGameplayEffectSpecHandle SpecHandle =SourceASC->MakeOutgoingSpec(DamageInfo.DamageEffectClass,1.f,ContextHandle);

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")),-DamageInfo.Amount);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),AbilitySystemComponent);	
		// 다른 전투 연출과 겹치지 않을 때만 피격 몽타주 재생
		if (!AbilitySystemComponent->HasMatchingGameplayTag(
			FGameplayTag::RequestGameplayTag(FName("State.Attacking"))) &&
			!AbilitySystemComponent->HasMatchingGameplayTag(
				FGameplayTag::RequestGameplayTag(FName("State.Stunned"))) &&
			AttributeSet->GetHealth() > 0.f)
		{
			MulticastPlayHitMontage();
		}
	}
}

void ABaseEnemy::SetTargetToTower()
{
	if (!TargetTower) return;
	ChangeTarget(TargetTower);
}

void ABaseEnemy::ChangeTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget) return;
	CurrentTarget = NewTarget;
	if (!CurrentTarget)	return;

	MoveToTarget();
}

void ABaseEnemy::OnPlayerTowerRegistered(APlayerTower* Tower)
{
	if (!Tower) return;

	TargetTower = Tower;
	Tower->OnGameOver.AddUniqueDynamic(this,&ABaseEnemy::HandleDeath);
}

bool ABaseEnemy::CanStartAttack() const
{
	if (!CurrentTarget) return false;

	// 타워는 중심점이 아닌 가장 가까운 표면 기준으로 공격 거리 계산
	if (Cast<APlayerTower>(CurrentTarget))
	{
		const FBox TowerBounds = CurrentTarget->GetComponentsBoundingBox(false);
		const FVector ClosestPoint = TowerBounds.GetClosestPointTo(GetActorLocation());

		FVector ToTarget = ClosestPoint - GetActorLocation();
		ToTarget.Z = 0.f;

		const float Distance = ToTarget.Size();

		if (Distance > CombatStats.AttackRange) return false;
		if (Distance <= KINDA_SMALL_NUMBER) return true;

		const FVector DirToTarget = ToTarget.GetSafeNormal();
		const float DotResult = FVector::DotProduct(DirToTarget, GetActorForwardVector());
		const float AttackTriggerRangeCos = FMath::Cos(FMath::DegreesToRadians(CombatStats.AttackAngle));

		return DotResult >= AttackTriggerRangeCos;
	}

	// 플레이어는 기존의 공격 판정으로
	return IsInSector(CurrentTarget, CombatStats.AttackRange, CombatStats.AttackAngle);
}

// 추격 포기 여부
bool ABaseEnemy::ShouldGiveUpTarget() const
{
	if (!CurrentTarget) return false;
	if (Cast<APlayerTower>(CurrentTarget)) return false;

	return GetDistanceTo(CurrentTarget) > CombatStats.ChaseGiveUpRange;
}

void ABaseEnemy::MulticastPlayDeathMontage_Implementation()
{
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

void ABaseEnemy::MulticastPlayHitMontage_Implementation()
{
	if (HitMontage)
	{
		PlayAnimMontage(HitMontage);
	}
}
#pragma endregion

#pragma region UI
void ABaseEnemy::SetHUDVisible(bool bVisible)
{
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(bVisible);
	}
}
#pragma endregion

#pragma region Sound
void ABaseEnemy::MulticastPlayDeathSound_Implementation()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}
#pragma endregion