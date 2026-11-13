// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyRPGCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MyRPG.h"
#include "MyAbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.h"
#include "MyRPGGameState.h"
#include "MyWorldSubsystem.h"
#include "PlayerTower.h"
#include "BaseEnemy.h"
#include "WeaponTraceComponent.h"
#include "MyGameplayAbility_MeleeAttack.h"
#include "MyGamePlayTag.h"
#include "BossEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayFunctionLibrary.h"
#include "WaveStartTrriger.h"
#include "DrawDebugHelpers.h"
#include "MyGameHUDWidget.h"
#include "MyWaveUISystem.h"

#pragma region Engine Overrides

AMyRPGCharacter::AMyRPGCharacter()
{
	// 충돌 캡슐 설정
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 컨트롤러 회전은 카메라에만 사용
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 이동 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// 카메라 암 생성
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// 카메라 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// GAS 및 전투용 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
	WeaponTraceComponent = CreateDefaultSubobject<UWeaponTraceComponent>(TEXT("WeaponTraceComponent"));
}

UAbilitySystemComponent* AMyRPGCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// GAS 초기화 후 서버 전용 데이터 설정 및 로컬 HUD 초기화
void AMyRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (HasAuthority())
	{
		// Attribute 초기화와 Ability 부여는 서버에서만 처리
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitStats(UMyAttributeSet::StaticClass(), nullptr);

			const UMyAttributeSet* AS = Cast<const UMyAttributeSet>(AbilitySystemComponent->GetAttributeSet(UMyAttributeSet::StaticClass()));

			if (AS)
			{
				const_cast<UMyAttributeSet*>(AS)->InitHealth(100.f);
				const_cast<UMyAttributeSet*>(AS)->InitMaxHealth(100.f);
				const_cast<UMyAttributeSet*>(AS)->InitMana(50.f);
				const_cast<UMyAttributeSet*>(AS)->InitMaxMana(50.f);
				const_cast<UMyAttributeSet*>(AS)->InitAttackPower(10.f);
				const_cast<UMyAttributeSet*>(AS)->InitDefense(5.f);
				const_cast<UMyAttributeSet*>(AS)->OnDeath.AddDynamic(this, &AMyRPGCharacter::HandleDeath);
			}

			if (BasicAttackAbility)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BasicAttackAbility));
			}

			if (ChargeAttackAbility)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ChargeAttackAbility));
			}

			if (DodgeAbility)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DodgeAbility));
			}

			for (TSubclassOf<UGameplayAbility>& Ability : DefaultAbilities)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
			}
		}

		if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
		{
			GS->RegisterPlayer(this);
		}
	}

	// 자신의 HUD는 로컬 플레이어에서만 생성
	if (IsLocallyControlled())
	{
		InitializeHUD();
	}
}

#pragma endregion

#pragma region Input Action

void AMyRPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/* Jumping
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this,&ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&ACharacter::StopJumping);
		*/

		// Dodging
		EnhancedInputComponent->BindAction(DodgingAction, ETriggerEvent::Started, this, &AMyRPGCharacter::DoDodge);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyRPGCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMyRPGCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyRPGCharacter::Look);

		// Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AMyRPGCharacter::DoAttack);

		// ChargeAttack
		EnhancedInputComponent->BindAction(ChargeAttackAct, ETriggerEvent::Started, this, &AMyRPGCharacter::DoCharge);
		EnhancedInputComponent->BindAction(ChargeAttackAct, ETriggerEvent::Completed, this, &AMyRPGCharacter::DoChargeAttack);

		// Open Setting
		EnhancedInputComponent->BindAction(GameMenuAction, ETriggerEvent::Started, this, &AMyRPGCharacter::OpenGameMenu);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyRPGCharacter::Interact);
	}
	else
	{
		UE_LOG(LogMyRPG, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMyRPGCharacter::Move(const FInputActionValue& Value)
{
	// MovementLocked 상태에서는 이동 입력 차단
	if (AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(MyGameplayTag::State_MovementLocked))
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMyRPGCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMyRPGCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMyRPGCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMyRPGCharacter::DoJumpStart()
{
	Jump();
}

void AMyRPGCharacter::DoJumpEnd()
{
	StopJumping();
}

void AMyRPGCharacter::DoAttack()
{
	if (!AbilitySystemComponent || !BasicAttackAbility) return;

	// 카메라 방향(바라보는) 으로 회전
	FVector AttackDirection = FollowCamera->GetForwardVector();
	AttackDirection.Z = 0.f;
	AttackDirection.Normalize();

	SetActorRotation(AttackDirection.Rotation());

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(BasicAttackAbility);
	if (!Spec) return;

	// 이미 실행 중인 Ability라면 추가 입력만 전달
	if (Spec->IsActive())
	{
		Spec->InputPressed = true;
		AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
		return;
	}

	AbilitySystemComponent->TryActivateAbility(Spec->Handle);
}

void AMyRPGCharacter::DoCharge()
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (!AbilitySystemComponent || !ChargeAttackAbility) return;

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(ChargeAttackAbility);
	if (!Spec || Spec->IsActive()) return;

	Spec->InputPressed = true;

	if (!AbilitySystemComponent->TryActivateAbility(Spec->Handle))
	{
		Spec->InputPressed = false;
	}
}

void AMyRPGCharacter::DoChargeAttack()
{
	if (!AbilitySystemComponent || !ChargeAttackAbility) return;

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(ChargeAttackAbility);
	if (!Spec) return;

	Spec->InputPressed = false;

	if (Spec->IsActive())
	{
		AbilitySystemComponent->AbilitySpecInputReleased(*Spec);
	}
}

void AMyRPGCharacter::OpenGameMenu()
{
	if (!IsLocallyControlled()) return;
	if (IsValid(GameMenuWidget)) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController || !GameMenuWidgetClass) return;

	GameMenuWidget = CreateWidget<UUserWidget>(PlayerController, GameMenuWidgetClass);

	if (!GameMenuWidget) return;

	GameMenuWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GameMenuWidget->TakeWidget());

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;

	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
}

void AMyRPGCharacter::DoDodge()
{
	if (!AbilitySystemComponent || !DodgeAbility) return;

	AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbility);
}

#pragma endregion

#pragma region GAS

void AMyRPGCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerHealthBarWidget || !AttributeSet) return;

	const float MaxHealth = AttributeSet->GetMaxHealth();
	if (MaxHealth <= 0.f) return;

	PlayerHealthBarWidget->UpdateHealthBar(Data.NewValue / MaxHealth);
}

void AMyRPGCharacter::ApplyDamage(const FDamageInfo& DamageInfo)
{
	if (!HasAuthority()) return;
	if (!AbilitySystemComponent || !AttributeSet) return;
	if (!DamageInfo.Attacker) return;
	if (!DamageInfo.DamageEffectClass) return;
	if (!UGameplayFunctionLibrary::CanDamage(DamageInfo.Attacker, this)) return;

	// Dodge 상태에서는 피격 무시
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dodging"))))
	{
		return;
	}

	IAbilitySystemInterface* AttackerASI = Cast<IAbilitySystemInterface>(DamageInfo.Attacker);
	if (!AttackerASI) return;

	UAbilitySystemComponent* SourceASC = AttackerASI->GetAbilitySystemComponent();
	if (!SourceASC) return;

	// 방어력을 적용한 최종 데미지는 0 미만으로 내려가지 않도록 제한
	float FinalDamage = FMath::Max(0.f, DamageInfo.Amount - AttributeSet->GetDefense());

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(DamageInfo.Attacker);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageInfo.DamageEffectClass, 1.f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -FinalDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent);

		if (FinalDamage > 0.f)
		{
			ClientPlayHitSound();
		}
	}
}

#pragma endregion

#pragma region UI

void AMyRPGCharacter::ShowGameOverScreen()
{
	if (!IsLocallyControlled()) return;
	if (IsValid(GameOverUI)) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !GameMenuWidgetClass) return;

	GameOverUI = CreateWidget<UUserWidget>(PlayerController, GameMenuWidgetClass);
	if (!GameOverUI) return;

	GameOverUI->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GameOverUI->TakeWidget());

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
}

void AMyRPGCharacter::CloseGameMenu()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;

		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}

	if (IsValid(GameMenuWidget))
	{
		GameMenuWidget->RemoveFromParent();
		GameMenuWidget = nullptr;
	}
}

void AMyRPGCharacter::InitializeHUD()
{
	if (!PlayerHUDClass) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsLocalController()) return;

	UMyGameHUDWidget* GameHUD = CreateWidget<UMyGameHUDWidget>(PC, PlayerHUDClass);
	if (!GameHUD) return;

	PlayerHUDWidget = GameHUD;
	PlayerHUDWidget->AddToViewport();

	PlayerHealthBarWidget = Cast<UHealthBarWidget>(PlayerHUDWidget->GetWidgetFromName(TEXT("PlayerHealthBar")));
	TowerHealthBarWidget = Cast<UHealthBarWidget>(PlayerHUDWidget->GetWidgetFromName(TEXT("TowerHealthBar")));
	BossHealthBarWidget = Cast<UHealthBarWidget>(PlayerHUDWidget->GetWidgetFromName(TEXT("BossHealthBar")));

	if (BossHealthBarWidget)
	{
		BossHealthBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 플레이어 HP 변경 Delegate 연결
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UMyAttributeSet::GetHealthAttribute())
			.AddUObject(this, &AMyRPGCharacter::OnHealthChanged);
	}

	if (AttributeSet)
	{
		const float CurrentHealth = AttributeSet->GetHealth();
		const float MaxHealth = AttributeSet->GetMaxHealth();

		if (PlayerHealthBarWidget && MaxHealth > 0.f)
		{
			PlayerHealthBarWidget->UpdateHealthBar(CurrentHealth / MaxHealth);
		}
	}

	// 타워 및 보스 등록 이벤트 연결
	if (UMyWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<UMyWorldSubsystem>())
	{
		Subsystem->OnPlayerTowerRegistered.AddUObject(this, &AMyRPGCharacter::OnPlayerTowerRegistered);
		Subsystem->OnBossEnemyRegistered.AddUObject(this, &AMyRPGCharacter::OnBossRegistered);

		if (APlayerTower* Tower = Subsystem->GetPlayerTower())
		{
			OnPlayerTowerRegistered(Tower);
		}
	}

	// Wave UI는 LocalPlayer의 WaveUISystem에서 관리
	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UMyWaveUISystem* WaveUISystem = LocalPlayer->GetSubsystem<UMyWaveUISystem>())
		{
			WaveUISystem->RegisterHUD(GameHUD);
		}
	}
}

void AMyRPGCharacter::OnTowerHealthChanged(float NewPercent)
{
	if (!TowerHealthBarWidget) return;

	TowerHealthBarWidget->UpdateHealthBar(NewPercent);
}

// 보스 등장 시 HP 변경 Delegate를 연결하고 GameState의 Boss UI 상태 갱신
void AMyRPGCharacter::OnBossRegistered(ABossEnemy* Boss)
{
	if (!Boss) return;

	Boss->OnBossHealthChanged.RemoveAll(this);
	Boss->OnBossHealthChanged.AddUObject(this, &AMyRPGCharacter::OnBossHealthChanged);

	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->SetBossUI(true, 1.f);
	}
}

void AMyRPGCharacter::OnBossHealthChanged(float NewPercent)
{
	if (AMyRPGGameState* GS = GetWorld()->GetGameState<AMyRPGGameState>())
	{
		GS->SetBossUI(true, NewPercent);
	}
}

#pragma endregion

#pragma region Sound

void AMyRPGCharacter::ClientPlayHitSound_Implementation()
{
	if (HitReactionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitReactionSound, GetActorLocation());
	}
}

void AMyRPGCharacter::ClientPlayDeathSound_Implementation()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}

#pragma endregion

void AMyRPGCharacter::OnPlayerTowerRegistered(APlayerTower* Tower)
{
	if (!Tower) return;

	Tower->OnGameOver.RemoveDynamic(this, &AMyRPGCharacter::ShowGameOverScreen);
	Tower->OnGameOver.AddDynamic(this, &AMyRPGCharacter::ShowGameOverScreen);

	Tower->OnTowerHealthChanged.RemoveAll(this);
	Tower->OnTowerHealthChanged.AddUObject(this, &AMyRPGCharacter::OnTowerHealthChanged);

	OnTowerHealthChanged(Tower->GetDurabilityPercent());
}

void AMyRPGCharacter::PlayDeathMontage_Implementation()
{
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

void AMyRPGCharacter::HandleDeath()
{
	if (!HasAuthority()) return;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
		AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	}

	SetActorEnableCollision(false);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	PlayDeathMontage();
	ClientPlayDeathSound();
}

#pragma region Interaction

void AMyRPGCharacter::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Do Interact"));

	AWaveStartTrriger* Target = FindWaveStartTrigger();

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trigger Not Found"));
		return;
	}

	ServerInteract(Target);
}

void AMyRPGCharacter::ServerInteract_Implementation(AWaveStartTrriger* Target)
{
	if (!IsValid(Target)) return;

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	// 클라이언트에서 전달된 대상이 실제 상호작용 거리 안에 있는지 서버에서 검증
	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());

	if (DistanceSquared > FMath::Square(InteractionDistance + 100.f)) return;

	Target->Interact();
}

AWaveStartTrriger* AMyRPGCharacter::FindWaveStartTrigger() const
{
	const FVector TraceStart = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FVector TraceEnd = TraceStart + GetActorForwardVector() * InteractionDistance;

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	if (!bHit) return nullptr;

	return Cast<AWaveStartTrriger>(HitResult.GetActor());
}

#pragma endregion