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
#include "Engine/OverlapResult.h"

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
	m_bJumpPressed = true;
	if (!m_bIsWallGrabbing)
		Jump();
}

void AC_PlayerCharacter::jumpEnd(const FInputActionValue& sValue)
{
	m_bJumpPressed = false;
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
	UE_LOG(LogTemp, Warning, TEXT(">>> [Player] Parry Input Triggered"));

	if (AActor* pEnemy = getCurrentEnemy())
	{
		if (m_bIsLockOn)
			pEnemy = m_pCurrentLockOnTarget;

		if (UC_ParryComponent* ParryComp = pEnemy->GetComponentByClass<UC_ParryComponent>())
		{
			if (ParryComp->isCanParry())
			{
				IC_CombatInterface::Execute_tryParry(pEnemy, this);
				UE_LOG(LogTemp, Warning, TEXT("[Player] Parry SUCCESS on %s"), *pEnemy->GetName());
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Player] Parry Failed ? No parryable enemy"));
	
}

void AC_PlayerCharacter::lockOn(const FInputActionValue& sValue)
{
	if (m_pCurrentLockOnTarget)
	{
		m_pCurrentLockOnTarget = nullptr;
		return;
	}

	AC_CombatCharacter* pTarget = findLockOnTarget();

	if (pTarget)
	{
		m_pCurrentLockOnTarget = pTarget;
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
		ECC_GameTraceChannel3,
		Params
	);

	if (bHit)
	{
		if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(HitResult.GetActor()))
		{
			return pEnemy;
		}
	}

	DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Red, false, 1.f, 0, 2.f);

	return nullptr;
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

void AC_PlayerCharacter::checkWallTrace()
{
	FVector vStart = GetActorLocation();
	FVector vForward = GetActorForwardVector();
	FVector vEnd = vStart + vForward * 100.f;

	FHitResult HitResult{};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(this);

	bool bHit =
		GetWorld()->LineTraceSingleByChannel
		(
			HitResult,
			vStart,
			vEnd,
			ECC_GameTraceChannel4,
			Params
		);


	float fDistance = (HitResult.Location - GetActorLocation()).Size();
	const float fMinWallGrabDistance = 50.f;

	if (bHit && HitResult.Normal.Z < 0.3f && fDistance <= fMinWallGrabDistance && m_bJumpPressed)
	{
		m_bCanWallGrab = true;
		m_vWallNormal = HitResult.Normal;
	}
	else
	{
		m_bCanWallGrab = false;
	}
}

void AC_PlayerCharacter::setWallGrab(bool bEnable)
{
	if (bEnable)
	{
		m_bIsWallGrabbing = true;

		// 중력 제거
		GetCharacterMovement()->GravityScale = 0.f;

		// 속도 제거
		GetCharacterMovement()->StopMovementImmediately();

		// 공중제어 금지
		GetCharacterMovement()->AirControl = 0.f;

		FVector Push = -m_vWallNormal * 10.f;
		SetActorLocation(GetActorLocation() + Push);


	}
	else
	{
		m_bIsWallGrabbing = false;

		GetCharacterMovement()->GravityScale = 1.f;
		GetCharacterMovement()->AirControl = 0.5f;
	}
}

bool AC_PlayerCharacter::checkClimbableSurface()
{
	if (!m_bIsWallGrabbing)
		return false;

	FVector vWallTraceStart = GetActorLocation();
	FVector vWallTraceEnd = vWallTraceStart + (-m_vWallNormal * 50.f) + FVector(0.f, 0.f, 200.f);
	FHitResult WallHit{};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(this);

	bool bWallHit =
		GetWorld()->LineTraceSingleByChannel
		(
			WallHit,
			vWallTraceStart,
			vWallTraceEnd,
			ECC_GameTraceChannel4,
			Params
		);

	if (!bWallHit)
		return false;

	const float fCapsuleRadius = 34.f;
	const float fCapsuleHalfHeight = 88.f;

	float fWallHeight = WallHit.Location.Z - GetActorLocation().Z;

	FVector vLedgeStart = WallHit.Location + (-m_vWallNormal * 50.f) + FVector(0.f, 0.f, 50.f);
	FVector vLedgeEnd = vLedgeStart + FVector(0.f, 0.f, fCapsuleHalfHeight * 2.f);
	
	

	FHitResult bLedgeHit{};

	bool bHit =
		GetWorld()->SweepSingleByChannel
		(
			bLedgeHit,
			vLedgeStart,
			vLedgeEnd,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeCapsule(fCapsuleRadius, fCapsuleHalfHeight),
			Params
		);

	DrawDebugCapsule(GetWorld(), (vLedgeStart + vLedgeEnd) * 0.5f,
		fCapsuleHalfHeight, fCapsuleRadius,
		FQuat::Identity, FColor::Green, false, 0.1f, 0, 1.f);

	if (!bHit)
		return false;



	if (bLedgeHit.Normal.Z >= 0.1f)
	{
		m_vClimbLocation = bLedgeHit.Location;

		m_bUseDirectClimb = fWallHeight < 50.f;

		return true;
	}


	return false;
	
}

void AC_PlayerCharacter::startClimbUp()
{
	if (!m_bCanClimbUp)
		return;

	
	m_bCanClimbUp = false;
	m_bIsWallGrabbing = false;
	GetCharacterMovement()->GravityScale = 1.f;
	GetCharacterMovement()->AirControl = 0.5f;

	if (m_bUseDirectClimb)
	{
		SetActorLocation(m_vClimbLocation);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

		if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
		{
			pAnim->playUpToClimb();
		}
	}
	
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

bool AC_PlayerCharacter::isLockOn() const
{
	return m_bIsLockOn;
}

bool AC_PlayerCharacter::isWallGrab() const
{
	return m_bIsWallGrabbing;
}

bool AC_PlayerCharacter::tryExcuteEnemy() const
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

	// 락온 기능
	setLockOn(DeltaTime);

	if (GetCharacterMovement()->IsFalling())
	{
		// 벽 근처인지 체크만
		checkWallTrace();
	}
	else
	{
		m_bCanWallGrab = false;
	}

	// 벽 짚기 토글
	if (m_bCanWallGrab && !m_bIsWallGrabbing)
	{
		setWallGrab(true);
	}
	else if (m_bIsWallGrabbing && !m_bCanWallGrab)
	{
		setWallGrab(false);
		StopJumping();
	}

	// 벽 짚은 상태 로직
	if (m_bIsWallGrabbing)
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		GetCharacterMovement()->GravityScale = 0.f;

		if (checkClimbableSurface())
		{
			m_bCanClimbUp = true;
			startClimbUp();
		}
	}


	
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
		pEinputCom->BindAction(m_pLockOnAction, ETriggerEvent::Started, this, &AC_PlayerCharacter::lockOn);
	}
}
