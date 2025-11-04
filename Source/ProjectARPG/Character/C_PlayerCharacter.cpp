// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Animation/C_PlayerAnim.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "Components/SphereComponent.h"

AC_PlayerCharacter::AC_PlayerCharacter()
{
	m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_pSpringArm->SetupAttachment(RootComponent);
	m_pSpringArm->TargetArmLength = 500.f;
	m_pSpringArm->bUsePawnControlRotation = true;

	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_pCamera->SetupAttachment(m_pSpringArm);
	m_pCamera->bUsePawnControlRotation = false;

	m_pExecutionDetectSphere = nullptr;
}

void AC_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 800.f, 0.f);

	if (APlayerController* pPlayerCon = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* pSubSystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pPlayerCon->GetLocalPlayer()))
		{
			pSubSystem->AddMappingContext(m_pMappingContext, 0);
		}
	}

	m_pExecutionDetectSphere = GetComponentByClass<USphereComponent>();

}



void AC_PlayerCharacter::look(const FInputActionValue& sValue)
{
	const FVector2D vLookAxis = sValue.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(vLookAxis.X);
		AddControllerPitchInput(vLookAxis.Y);
	}
}

void AC_PlayerCharacter::move(const FInputActionValue& sValue)
{
	const FVector2D vMoveDir = sValue.Get<FVector2D>();

	if (Controller)
	{
		const auto rot = Controller->GetControlRotation();
		const FRotator rYawRot(0, rot.Yaw, 0);
		const auto vForwardDir = FRotationMatrix(rYawRot).GetUnitAxis(EAxis::X);
		const auto vRightDir = FRotationMatrix(rYawRot).GetUnitAxis(EAxis::Y);

		AddMovementInput(vForwardDir, vMoveDir.Y);
		AddMovementInput(vRightDir, vMoveDir.X);
	}
}

void AC_PlayerCharacter::sprint(const FInputActionInstance& sInst)
{

	UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance());

	if (!pAnim)
		return;

	
	if (sInst.GetTriggerEvent() != ETriggerEvent::Triggered)
		return;	

	if (m_eState != E_CombatState::Idle)
		return;


	const float fElapsedTime = sInst.GetElapsedTime();


	const float fHoldThreshold = 0.3f;


	if (fElapsedTime >= fHoldThreshold)
	{
		if (GetLastMovementInputVector().IsNearlyZero())
			return;
		
		// 대시 실행
		if (m_eState == E_CombatState::Idle)
		{			
			m_eState = E_CombatState::Sprinting;

			pAnim->playSprintStartMontage();

			if (APlayerController* pPlayerCon = Cast<APlayerController>(GetController()))
			{
				if (AC_PlayerCameraManager* pCameraMgr = Cast<AC_PlayerCameraManager>(pPlayerCon->PlayerCameraManager))
				{
					pCameraMgr->startSprintEffect();
				}
			}
			FVector vForwardDir = GetActorForwardVector();
			FVector vLaunchVelocity = vForwardDir * 1000.f; // 숫자 조절해서 속도/거리 조정

			LaunchCharacter(vLaunchVelocity, true, false);
			GetCharacterMovement()->MaxWalkSpeed = 1000.f;
		}	
		
	}
	else
	{

		m_eState = E_CombatState::Dodging;


		// 회피 실행
		FVector vInputDir = GetLastMovementInputVector().GetSafeNormal();

		FVector vForward = GetActorForwardVector();
		FVector vRight = GetActorRightVector();

		float fForwardDot = FVector::DotProduct(vForward, vInputDir);
		float fRightDot = FVector::DotProduct(vRight, vInputDir);

		E_Direction eDir = E_Direction::Backward;

		if (!vInputDir.IsNearlyZero())
		{
			if (FMath::Abs(fForwardDot) > FMath::Abs(fRightDot))
			{
				eDir = (fForwardDot > 0) ? E_Direction::Forward : E_Direction::Backward;
			}
			else
			{
				eDir = (fRightDot > 0) ? E_Direction::Right : E_Direction::Left;
			}
		}


		pAnim->playDodgeMontage(eDir);

		GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
		
		
	}

	
}

void AC_PlayerCharacter::sprintReleased(const FInputActionInstance& sInst)
{
	if (m_eState != E_CombatState::Sprinting)
		return;

	if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying() && m_eState == E_CombatState::Sprinting)
		GetMesh()->GetAnimInstance()->Montage_Stop(0.1f);

	m_eState = E_CombatState::Idle;
	GetCharacterMovement()->MaxWalkSpeed = 800.f;
	if (APlayerController* pPlayerCon = Cast<APlayerController>(GetController()))
	{
		if (AC_PlayerCameraManager* pCameraMgr = Cast<AC_PlayerCameraManager>(pPlayerCon->PlayerCameraManager))
		{
			pCameraMgr->stopSprintEffect();
		}
	}
}

void AC_PlayerCharacter::jumpStart(const FInputActionValue& sValue)
{
	Jump();
}

void AC_PlayerCharacter::jumpEnd(const FInputActionValue& sValue)
{
	StopJumping();
}

void AC_PlayerCharacter::guard(const FInputActionInstance& sInst)
{
	UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance());

	if (!pAnim)
		return;

	FVector vInputDir = GetLastMovementInputVector().GetSafeNormal();

	const float fElapsedTime = sInst.GetElapsedTime();

	const float fHoldThreshold = 0.3f;

	if (fElapsedTime >= fHoldThreshold && GetLastMovementInputVector().IsNearlyZero())
	{
		if (m_eState != E_CombatState::Guard)
		{
			m_eState = E_CombatState::Guard;
			pAnim->setIsGuarding(true);
			GetCharacterMovement()->MaxWalkSpeed = 400.f;
		}
	}
	
}

void AC_PlayerCharacter::guardEnd(const FInputActionValue& sValue)
{
	if (m_eState == E_CombatState::Guard)
	{
		m_eState = E_CombatState::Idle;
		if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
		{
			pAnim->setIsGuarding(false);
			GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
		}
	}
}

void AC_PlayerCharacter::parry(const FInputActionValue& sValue)
{
	UE_LOG(LogTemp, Warning, TEXT(">>> [Player] Parry Input Function CALLED!"));

	AActor* Attacker = getCurrentEnemy();
	if (m_pParryCom)
	{
		if (m_pParryCom->tryParry(Attacker))
		{
			UE_LOG(LogTemp, Warning, TEXT("Parry SUCCESS on %s"), *Attacker->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Parry Failed"));
		}
	}
	
}


void AC_PlayerCharacter::comboAttack(const FInputActionValue& sValue)
{
	m_fLastAttackInputTime = GetWorld()->GetTimeSeconds();

	if (tryExcuteEnemy())
		return;

	if (m_eState == E_CombatState::Sprinting || m_eState == E_CombatState::Dodging)
	{
		// 대시 상태 해제
		setCombatState(E_CombatState::Attacking);


		m_nCurrentComboIndex = 1;
		m_bNextComboQueued = false;
		playCombo(m_nCurrentComboIndex);
		return;
	}

	if (m_eState != E_CombatState::Attacking)
	{
		m_eState = E_CombatState::Attacking;
		m_nCurrentComboIndex = 1;
		m_bNextComboQueued = false;
		playCombo(m_nCurrentComboIndex);
	}

	else if (m_eState == E_CombatState::Attacking && !m_bNextComboQueued)
	{
		if (m_nCurrentComboIndex < m_nMaxComboIndex)
			m_bNextComboQueued = true;
	}



}


void AC_PlayerCharacter::playCombo(int32 nComboIndex)
{
	m_nCurrentComboIndex = nComboIndex;
	
	if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] AnimInstance cast SUCCESS"));
		m_eAttackType = E_AttackType::Normal;
		pAnim->playComboMontage(m_eAttackType,nComboIndex);
	}

}

AActor* AC_PlayerCharacter::getCurrentEnemy()
{
	FVector vStart = GetActorLocation();
	FVector vForward = GetActorForwardVector();
	FVector vEnd = vStart + vForward * 400.f;

	FHitResult HitResult{};

	FCollisionQueryParams Params{};

	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		vStart,
		vEnd,
		ECC_Pawn,
		Params
	);

	if (bHit)
	{
		AActor* pHitActor = HitResult.GetActor();
		if (pHitActor) // 태그로 적 판정
		{
			return pHitActor;
		}
	}

	DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Red, false, 1.f, 0, 2.f);

	return nullptr;
}

void AC_PlayerCharacter::setCombatState(E_CombatState eNewState)
{
	Super::setCombatState(eNewState);

	if (m_eState == E_CombatState::Idle)
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
	}
}

void AC_PlayerCharacter::onComboTransition()
{
	if (m_bNextComboQueued && GetWorld()->GetTimeSeconds() - m_fLastAttackInputTime <= m_fInputBuffer)
	{
		m_bNextComboQueued = false;
		m_nCurrentComboIndex++;

		if (m_nCurrentComboIndex > m_nMaxComboIndex)
		{
			resetCombo();
			return;
		}

		playCombo(m_nCurrentComboIndex);
	}
	else
	{
		resetCombo();
	}
}

void AC_PlayerCharacter::resetCombo()
{
	m_eState = E_CombatState::Idle;
	m_nCurrentComboIndex = 0;
	m_bNextComboQueued = false;
}

bool AC_PlayerCharacter::tryExcuteEnemy()
{
	TArray<AActor*> Overlaps{};

	if (m_pExecutionDetectSphere)
		m_pExecutionDetectSphere->GetOverlappingActors(Overlaps, AC_EnemyCharacter::StaticClass());

	UE_LOG(LogTemp, Warning, TEXT("Overlapping Count: %d"), Overlaps.Num());

	for (AActor* pAct : Overlaps)
	{
		if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pAct))
		{
			if (pEnemy->canBeExecuted())
			{
				if (m_pExecutionCom)
				{
					m_pExecutionCom->triggerExecution(pEnemy);
					UE_LOG(LogTemp, Error, TEXT("TriggerExecution!!!"));
					return true;
				}
			}

		}

	}
	return false;
}


void AC_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* pEinputCom = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		pEinputCom->BindAction(m_pLookAction, ETriggerEvent::Triggered, this, &AC_PlayerCharacter::look);
		pEinputCom->BindAction(m_pMoveAction, ETriggerEvent::Triggered, this, &AC_PlayerCharacter::move);
		pEinputCom->BindAction(m_pSprintAction, ETriggerEvent::Triggered, this, &AC_PlayerCharacter::sprint);
		pEinputCom->BindAction(m_pSprintAction, ETriggerEvent::Completed, this, &AC_PlayerCharacter::sprintReleased);
		pEinputCom->BindAction(m_pComboAttackAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::comboAttack);
		pEinputCom->BindAction(m_pJumpAction, ETriggerEvent::Triggered, this, &AC_PlayerCharacter::jumpStart);
		pEinputCom->BindAction(m_pJumpAction, ETriggerEvent::Completed, this, &AC_PlayerCharacter::jumpEnd);
		pEinputCom->BindAction(m_pGuardAction, ETriggerEvent::Triggered, this, &AC_PlayerCharacter::guard);
		pEinputCom->BindAction(m_pGuardAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::parry);
		pEinputCom->BindAction(m_pGuardAction, ETriggerEvent::Completed, this, &AC_PlayerCharacter::guardEnd);
	}
}
