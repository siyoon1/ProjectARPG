// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Animation/C_PlayerAnim.h"
#include "ProjectARPG/ActorComponents/C_GrappleComponent.h"
#include "ProjectARPG/ActorComponents/C_InteractionComponent.h"
#include "ProjectARPG/ActorComponents/C_MoveActionComponent.h"
#include "ProjectARPG/ActorComponents/C_PlayerExecutionComponent.h"
#include "ProjectARPG/Enums/C_ExecutionTypes.h"
#include "ProjectARPG/Data/C_PlayerExecutionData.h"
#include "ProjectARPG/ActorComponents/C_AttackComponent.h"
#include "ProjectARPG/ActorComponents/C_LockOnComponent.h"



AC_PlayerCharacter::AC_PlayerCharacter()
{
	m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_pSpringArm->SetupAttachment(RootComponent);
	m_pSpringArm->TargetArmLength = 500.f;
	m_pSpringArm->bUsePawnControlRotation = true;

	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_pCamera->SetupAttachment(m_pSpringArm);
	m_pCamera->bUsePawnControlRotation = false;

	m_ExecutionComp = CreateDefaultSubobject<UC_PlayerExecutionComponent>(TEXT("ExecutionComp"));
	m_LockOnComp = CreateDefaultSubobject<UC_LockOnComponent>(TEXT("LockOnComp"));

	m_pExecutionDetectSphere = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("[Init] ActionState=%d"), (int)m_ActionState);
}

void AC_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = m_fDefaultCrouched;
	GetCharacterMovement()->MaxAcceleration = m_fDefaultAcceleration;
	GetCharacterMovement()->BrakingDecelerationWalking = m_fDefaultBraking;
	GetCharacterMovement()->GravityScale = m_fDefaultGravity;
	GetCharacterMovement()->JumpZVelocity = m_fDefaultJumpVelocity;
	GetCharacterMovement()->AirControl = m_fDefaultAirControl;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	if (APlayerController* pPlayerCon = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* pSubSystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pPlayerCon->GetLocalPlayer()))
		{
			pSubSystem->AddMappingContext(m_pMappingContext, 0);
		}
	}

	m_pExecutionDetectSphere = GetComponentByClass<USphereComponent>();

	m_pGrappleCom = GetComponentByClass<UC_GrappleComponent>();

	m_pInteractCom = GetComponentByClass<UC_InteractionComponent>();

	m_pMoveActionCom = GetComponentByClass<UC_MoveActionComponent>();

	if (m_pMoveActionCom)
		m_pMoveActionCom->init(this);

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;


}

void AC_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// 락온 기능
	if (m_LockOnComp && m_LockOnComp->isLockOn())
	{
		applyLockOnRotation(DeltaTime);
	}
	else
	{
		releaseLockOnState();
	}

}

bool AC_PlayerCharacter::isInvincibleAgainst(AActor* pAttacker) const
{
	if (m_eState == E_CombatState::Executing)
	{
		return true;
	}

	return Super::isInvincibleAgainst(pAttacker);
}

void AC_PlayerCharacter::onActionFinished()
{
	m_ActionState = E_ActionState::Free;
	m_CombatMode = E_CombatMode::None;
	setCombatState(E_CombatState::Idle);
	initJump();
}

void AC_PlayerCharacter::onExecutionFinished()
{
	onActionFinished();

	EnableInput(nullptr);

	GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
}

void AC_PlayerCharacter::setCombatState(E_CombatState eNewState)
{
	Super::setCombatState(eNewState);

	if (m_eState == E_CombatState::Idle)
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
	}
}


E_CombatState AC_PlayerCharacter::getCombatState() const
{
	return m_eState;
}

float AC_PlayerCharacter::getDefaultGravity() const
{
	return m_fDefaultGravity;
}

float AC_PlayerCharacter::getDefaultAirControl() const
{
	return m_fDefaultAirControl;
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
	FVector2D vMoveDir = sValue.Get<FVector2D>();

	if (m_pMoveActionCom && m_pMoveActionCom->isWallGrabbing())
	{
		m_pMoveActionCom->wallMove(vMoveDir);
		return;
	}

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
	if (m_pMoveActionCom)
		m_pMoveActionCom->onSprintInput(sInst);
	
}



void AC_PlayerCharacter::jumpStart(const FInputActionValue& sValue)
{
	m_bJumpPressed = true;

	if (isCrouch())
		return;

	if (m_pMoveActionCom && m_pMoveActionCom->tryWallJumpOrClimb())
	{
		m_nJumpCount = 0;
		return;
	}

	if (m_nJumpCount == 0 && GetCharacterMovement()->IsMovingOnGround() && m_eState != E_CombatState::Climb)
	{
		Jump();
		m_nJumpCount++;
	}
	// 2단 점프 (공중)
	else if (!GetCharacterMovement()->IsMovingOnGround() && m_nJumpCount < m_MaxJumpCount)
	{
		LaunchCharacter(FVector(0, 0, 600.f), false, true);
		m_nJumpCount++;
	}
	
}

void AC_PlayerCharacter::jumpEnd(const FInputActionValue& sValue)
{
	m_bJumpPressed = false;
	StopJumping();
}

void AC_PlayerCharacter::guard(const FInputActionInstance& sInst)
{
	interruptMoveAction();

	if (!canAct())
		return;

	const float fElapsedTime = sInst.GetElapsedTime();

	const float fHoldThreshold = 0.3f;

	if (fElapsedTime < fHoldThreshold)
		return;

	if (!GetLastMovementInputVector().IsNearlyZero())
		return;

	if (!startGuard())
		return;

	UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance());

	if (!pAnim)
		return;

	pAnim->setIsGuarding(true);

	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	
	
}

void AC_PlayerCharacter::crouch(const FInputActionValue& sValue)
{
	if (m_pMoveActionCom && m_pMoveActionCom->isWallGrabbing())
	{
		m_pMoveActionCom->interruptMovementAction();
		return;
	}

	// 일반 앉기 토글
	if (!m_bIsCrouch)
	{
		Crouch();
		m_bIsCrouch = true;
		setCombatState(E_CombatState::Crouch);
		GetCharacterMovement()->MaxWalkSpeed = 250.f;
	}
	else
	{
		UnCrouch();
		m_bIsCrouch = false;
		setCombatState(E_CombatState::Idle);
		GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
	}
}

void AC_PlayerCharacter::interact(const FInputActionValue& sValue)
{
	if (m_pInteractCom && m_pInteractCom->tryInteract())
		return;

	if (m_pMoveActionCom)
	{
		m_pMoveActionCom->tryWallGrab();
	}
}

void AC_PlayerCharacter::grapple(const FInputActionValue& sValue)
{
	if (m_pGrappleCom)
		m_pGrappleCom->tryStartGrapple();
}

FName AC_PlayerCharacter::getComboAttackRow(int32 ComboIndex) const
{
	switch (ComboIndex)
	{
	case 1: return TEXT("Player_Normal_1");
	case 2: return TEXT("Player_Normal_2");
	case 3: return TEXT("Player_Normal_3");
	case 4: return TEXT("Player_Normal_4");
	default: return NAME_None;
	}
	
}

void AC_PlayerCharacter::guardEnd(const FInputActionValue& sValue)
{
	if (!isGuard())
		return;

	endGuard();


	if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->setIsGuarding(false);
	}

	GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
}

void AC_PlayerCharacter::parry(const FInputActionValue& sValue)
{
	interruptMoveAction();

	if (!canAct())
		return;

	if (m_eState != E_CombatState::Idle)
		return;

	if (!m_ParryCom)
		return;

	enterCombatMode(E_CombatMode::Parrying, E_ActionState::Locked);
	setCombatState(E_CombatState::Parrying);

	m_ParryCom->startParryWindow(0.25f);

	//AC_CombatCharacter* Target = nullptr;

	/*if (m_pCurrentLockOnTarget)
	{
		Target = Cast<AC_CombatCharacter>(m_pCurrentLockOnTarget);
	}
	else
	{
		Target = Cast<AC_CombatCharacter>(getCurrentEnemy());
	}

	if (!Target)
		return;*/

}

void AC_PlayerCharacter::lockOn(const FInputActionValue& sValue)
{
	if (m_LockOnComp)
	{
		m_LockOnComp->toggleLockOn();
	}
}


void AC_PlayerCharacter::comboAttack(const FInputActionValue& sValue)
{
	interruptMoveAction();
	m_fLastAttackInputTime = GetWorld()->GetTimeSeconds();

	if (tryStartExecution())
		return;


	if (m_ActionState == E_ActionState::Locked &&
		m_CombatMode == E_CombatMode::Attacking)
	{
		if (!m_bNextComboQueued &&
			m_nCurrentComboIndex < m_nMaxComboIndex)
		{
			m_bNextComboQueued = true;

			UE_LOG(LogTemp, Warning, TEXT("Combo Queued"));
		}
		return;
	}

	

	if (m_ActionState != E_ActionState::Free)
		return;

	if (!canAttack())
		return;

	startAttackCombo();

}


void AC_PlayerCharacter::playCombo(int32 nComboIndex)
{
	m_nCurrentComboIndex = nComboIndex;

	setCurrentAttackRow(getComboAttackRow(nComboIndex));
	
	if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->playComboMontage(nComboIndex);
	}

}

bool AC_PlayerCharacter::canAttack() const
{
	if (m_eState == E_CombatState::WallGrabbing ||
		m_eState == E_CombatState::Climb ||
		m_eState == E_CombatState::Die)
		return false;

	return true;
}

void AC_PlayerCharacter::startAttackCombo()
{
	m_ActionState = E_ActionState::Locked;
	m_CombatMode = E_CombatMode::Attacking;

	setCombatState(E_CombatState::Attacking);

	m_nCurrentComboIndex = 1;
	m_bNextComboQueued = false;

	playCombo(m_nCurrentComboIndex);
}

bool AC_PlayerCharacter::canExecute() const
{
	if (m_ActionState != E_ActionState::Free)
		return false;

	if (m_eState == E_CombatState::Climb ||
		m_eState == E_CombatState::Die)
		return false;

	return true;
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
		ECC_GameTraceChannel3,
		Params
	);

	if (bHit)
	{
		if (AC_CombatCharacter* pEnemy = Cast<AC_CombatCharacter>(HitResult.GetActor()))
		{
			return pEnemy;
		}
	}

	DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Red, false, 1.f, 0, 2.f);

	return nullptr;
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
	if (m_CombatMode == E_CombatMode::Attacking)
	{
		m_ActionState = E_ActionState::Free;
		m_CombatMode = E_CombatMode::None;

		setCombatState(E_CombatState::Idle);
	}
	


	m_nCurrentComboIndex = 0;
	m_bNextComboQueued = false;
}

USphereComponent* AC_PlayerCharacter::getExecutionSphere() const
{
	if (m_pExecutionDetectSphere)
		return m_pExecutionDetectSphere;

	return nullptr;
}

void AC_PlayerCharacter::restoreHP()
{
	//m_fCurrentHp = m_fMaxHp;
}

void AC_PlayerCharacter::resetPosture()
{
	//if (m_fCurrentPosture > 0)
		//m_fCurrentPosture = m_fMaxPosture;
}

void AC_PlayerCharacter::onParried_Implementation(AActor* ParryOwner)
{
	if (m_AttackComp)
	{
		m_AttackComp->endAttack();
	}

	enterCombatMode(E_CombatMode::None, E_ActionState::Free);
}

void AC_PlayerCharacter::onParrySuccess_Implementation(AActor* ParryTarget, E_ParryDirection Direction)
{
	enterCombatMode(E_CombatMode::None, E_ActionState::Free);
	setCombatState(E_CombatState::Idle);

	if (UC_PlayerAnim* Anim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
	{
		Anim->playParrySuccessMontage(Direction);
	}

	if (m_CamMgr)
	{
		applyHitStop(0.9f, 0.2f);
		m_CamMgr->playHitCameraShake(0.5f);
	}
		
}

void AC_PlayerCharacter::playPlayerExecutionMontage(const FS_ExecutionContext& Context)
{
	UAnimMontage* Montage =
		m_PlayerExecutionData->selectMontage(
			Context.Type,
			Context.Index);

	if (Montage)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			applyExecutionWarp(Context);


			Anim->Montage_Play(Montage);

			float Length = Montage->GetPlayLength();

			m_CamMgr->executionEffect(Length * 0.5f);
		}
	}
}

AC_CombatCharacter* AC_PlayerCharacter::findLockOnTarget()
{
	const float fDetectRadius = 500.f;

	FVector vCenter = GetActorLocation();

	TArray<FOverlapResult> listResult{};

	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(this);

	bool bHasHit = GetWorld()->OverlapMultiByChannel(
		listResult,
		vCenter,
		FQuat::Identity,
		ECC_GameTraceChannel3,
		FCollisionShape::MakeSphere(fDetectRadius),
		Params
		);

	DrawDebugSphere(GetWorld(), vCenter, fDetectRadius, 16, FColor::Red, false, 1.0f);

	AC_CombatCharacter* pTarget = nullptr;

	if (bHasHit)
	{
		
		for (const FOverlapResult& object : listResult)
		{
			pTarget = Cast<AC_CombatCharacter>(object.GetActor());
			
			if (!pTarget || pTarget == this)
				continue;

		}

	}
	else
		return nullptr;

	if (pTarget)
		UE_LOG(LogTemp, Warning, TEXT("LockOn Target: %s"), *pTarget->GetName());

	return pTarget;
}

void AC_PlayerCharacter::setLockOn(float fDelta)
{
	if (!m_pCurrentLockOnTarget)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;
		bUseControllerRotationPitch = false;
		m_bIsLockOn = false;
		return;
	}

	m_bIsLockOn = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
	bUseControllerRotationPitch = true;

	FVector vTargetLoc = m_pCurrentLockOnTarget->GetActorLocation();
	FVector vCameraLoc = m_pSpringArm->GetComponentLocation();
	FVector vDir = (vTargetLoc - vCameraLoc).GetSafeNormal();

	FRotator rTargetRot = vDir.Rotation();

	rTargetRot.Pitch -= 15.f;

	FRotator rNewRot = FMath::RInterpTo(m_pSpringArm->GetComponentRotation(), rTargetRot, fDelta, 3.f);

	Controller->SetControlRotation(rNewRot);
}


bool AC_PlayerCharacter::isLockOn() const
{
	if (!m_LockOnComp)
		return false;

	return m_LockOnComp->isLockOn();
}

bool AC_PlayerCharacter::canGrabWallAtLoc(const FVector& checkLoc)
{
	float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 벽 앞에서 상체 정도 높이에서 트레이스
	FVector Start = checkLoc + FVector(0, 0, HalfHeight * 0.7f);
	FVector End = Start + GetActorForwardVector() * 80.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_GameTraceChannel4, Params
	);

	const float fMinGrabDistance = 80.f;

	return (bHit && Hit.Normal.Z < 0.5f &&
		FVector::Dist(Start, Hit.Location) <= fMinGrabDistance);

}

void AC_PlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	m_nJumpCount = 0;
}


bool AC_PlayerCharacter::isWallGrab() const
{
	return m_pMoveActionCom && m_pMoveActionCom->isWallGrabbing();
}

bool AC_PlayerCharacter::isCrouch() const
{
	return m_bIsCrouch;
}

bool AC_PlayerCharacter::isCanWallGrab() const
{
	return m_pMoveActionCom&& m_pMoveActionCom->canWallGrab();
}

bool AC_PlayerCharacter::isWallGrabbing() const
{
	return  m_pMoveActionCom && m_pMoveActionCom->isWallGrabbing();
}

bool AC_PlayerCharacter::isPulling() const
{
	if (m_pGrappleCom)
		return m_pGrappleCom->isPulling();

	return false;
}

bool AC_PlayerCharacter::isPlayerControlled() const
{
	return Cast<APlayerController>(GetController()) != nullptr;
}

UCameraComponent* AC_PlayerCharacter::getFollowCamera() const
{
	return m_pCamera;
}

void AC_PlayerCharacter::initJump()
{
	m_nJumpCount = 0;
}

bool AC_PlayerCharacter::tryStartExecution()
{
	if (m_ActionState != E_ActionState::Free)
		return false;

	if (!m_ExecutionComp)
		return false;

	if (!m_ExecutionComp->canStartExecution())
		return false;

	m_ActionState = E_ActionState::Locked;
	m_CombatMode = E_CombatMode::Executing;
	setCombatState(E_CombatState::Executing);

	return m_ExecutionComp->tryExecuteCurrentTarget();
}

void AC_PlayerCharacter::applyExecutionWarp(const FS_ExecutionContext& Context)
{
	AActor* Victhim = Context.Victim.Get();

	if (!Victhim)
		return;

	if (Context.Type == E_ExecutionType::Stealth)
		return;

	const FRotator TargetRot = (Victhim->GetActorLocation() - GetActorLocation()).Rotation();

	SetActorRotation(TargetRot);
}

void AC_PlayerCharacter::applyLockOnRotation(float DeltaTime)
{
	FRotator TargetRot;
	if (!m_LockOnComp->getLockOnRotation(TargetRot))
		return;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	FRotator NewRot = FMath::RInterpTo(
		Controller->GetControlRotation(),
		TargetRot,
		DeltaTime,
		3.f
	);

	Controller->SetControlRotation(NewRot);
}

void AC_PlayerCharacter::releaseLockOnState()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void AC_PlayerCharacter::interruptMoveAction()
{
	if (m_pMoveActionCom)
	{
		m_pMoveActionCom->interruptMovementAction();
	}
}

void AC_PlayerCharacter::requestExecution_Implementation(AActor* Target)
{
	UE_LOG(LogTemp, Error, TEXT("requestExecution called"));

	if(!Target)
		return;

	APawn* VictimPawn = Cast<APawn>(Target);
	if (!VictimPawn)
		return;

	if (m_ExecutionComp)
		m_ExecutionComp->forceExecute(this, VictimPawn, E_ExecutionType::PostureBreak);
}

void AC_PlayerCharacter::sprintReleased(const FInputActionInstance& sInst)
{
	if (m_pMoveActionCom)
		m_pMoveActionCom->onSprintReleased();
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
		pEinputCom->BindAction(m_pJumpAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::jumpStart);
		pEinputCom->BindAction(m_pJumpAction, ETriggerEvent::Completed, this, &AC_PlayerCharacter::jumpEnd);
		pEinputCom->BindAction(m_pGuardAction, ETriggerEvent::Triggered, this, &AC_PlayerCharacter::guard);
		pEinputCom->BindAction(m_pGuardAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::parry);
		pEinputCom->BindAction(m_pGuardAction, ETriggerEvent::Completed, this, &AC_PlayerCharacter::guardEnd);
		pEinputCom->BindAction(m_pLockOnAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::lockOn);
		pEinputCom->BindAction(m_pCrouchAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::crouch);
		pEinputCom->BindAction(m_pInteractAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::interact);
		pEinputCom->BindAction(m_pGrappleAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::grapple);
	}
}
