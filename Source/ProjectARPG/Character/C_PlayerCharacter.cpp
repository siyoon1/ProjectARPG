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
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Animation/C_PlayerAnim.h"

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

void AC_PlayerCharacter::setCombatState(E_CombatState eNewState)
{
	Super::setCombatState(eNewState);

	if (m_eState == E_CombatState::Idle)
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
	}
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

	if (isCrouch())
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
			setCombatState(E_CombatState::Sprinting);

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

	if (isCrouch())
		return;

	if (m_bIsWallGrabbing)
	{
		FVector ClimbPos = checkClimbableSurface();

		if (ClimbPos != FVector::ZeroVector)
		{
			m_vClimbLocation = ClimbPos;
			startClimbUp();
		}
		return;
	}

	if (m_nJumpCount == 0 && GetCharacterMovement()->IsMovingOnGround())
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

void AC_PlayerCharacter::crouch(const FInputActionValue& sValue)
{
	if (!m_bIsCrouch)
	{
		m_bIsCrouch = true;
		setCombatState(E_CombatState::Crouch);
		GetCharacterMovement()->MaxWalkSpeed = 250.f;
	}
		
	else
	{
		m_bIsCrouch = false;
		setCombatState(E_CombatState::Idle);
		GetCharacterMovement()->MaxWalkSpeed = m_fDefaultSpeed;
	}
		
}

void AC_PlayerCharacter::interact(const FInputActionValue& sValue)
{
	if (isCanWallGrab())
	{
		setWallGrab(true);
	}
}

void AC_PlayerCharacter::guardEnd(const FInputActionValue& sValue)
{
	if (m_eState == E_CombatState::Guard)
	{
		setCombatState(E_CombatState::Idle);
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

	if (!canAttack())
		return;

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

bool AC_PlayerCharacter::canAttack() const
{
	switch (m_eState)
	{
	case E_CombatState::Sprinting:
	case E_CombatState::Dodging:
	case E_CombatState::Executing:
	case E_CombatState::Guard:
	case E_CombatState::Parrying:
	case E_CombatState::Climb:
	case E_CombatState::Crouch:
	case E_CombatState::Die:
		return false;
	default:
		return true;
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
	return m_bIsLockOn;
}

void AC_PlayerCharacter::checkWallTrace()
{
	if (!m_bCanWallJump)
		return;

	float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 벽 앞에서 상체 정도 높이에서 트레이스
	FVector Start = GetActorLocation() + FVector(0, 0, HalfHeight * 0.7f);
	FVector End = Start + GetActorForwardVector() * 80.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_GameTraceChannel4, Params
	);

	const float fMinGrabDistance = 60.f;

	if (bHit && Hit.Normal.Z < 0.5f &&
		FVector::Dist(Start, Hit.Location) <= fMinGrabDistance)
	{
		m_bCanWallGrab = true;

		// 새로 벽 붙을 때만 노멀 갱신
		if (!m_bIsWallGrabbing)
			m_vWallNormal = Hit.Normal;
	}
	else
	{
		m_bCanWallGrab = false;

		// 벽잡기 상태 유지 중이면 해제
		if (m_bIsWallGrabbing)
			setWallGrab(false);
	}
}

void AC_PlayerCharacter::setWallGrab(bool bEnable)
{
	if (bEnable)
	{
		m_bIsWallGrabbing = true;

		// 중력 제거 + 속도 제거
		auto Move = GetCharacterMovement();
		Move->GravityScale = 0.f;
		Move->StopMovementImmediately();
		Move->AirControl = 0.f;

		bUseControllerRotationYaw = false;

		// 벽에 밀착
		SetActorLocation(GetActorLocation() - (-m_vWallNormal * 10.f));
	}
	else
	{
		m_bIsWallGrabbing = false;

		auto Move = GetCharacterMovement();
		Move->GravityScale = 1.f;
		Move->AirControl = 0.5f;

		bUseControllerRotationYaw = true;
	}
}

FVector AC_PlayerCharacter::checkClimbableSurface()
{

	if (!m_bIsWallGrabbing)
		return FVector::ZeroVector;

	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // 88
	const float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();        // 34

	FVector ActorLoc = GetActorLocation();

	//-------------------------------------------
	// 1) 위로 스윕 → 난간 윗면 찾기
	//-------------------------------------------
	FVector SweepStart =
		ActorLoc
		- m_vWallNormal * (Radius + 0.9f)    // 벽과 거의 밀착
		+ FVector(0, 0, HalfHeight * 0.7f); // 살짝 위에서 시작

	FVector SweepEnd = SweepStart - m_vWallNormal * 5.f + FVector(0, 0, HalfHeight * 0.5f);

	FHitResult HitUp;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bUp = GetWorld()->SweepSingleByChannel(
		HitUp,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_GameTraceChannel4,
		FCollisionShape::MakeCapsule(Radius * 0.8f, HalfHeight * 0.3f),
		Params
	);

	if (!bUp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sweep failed"));
		return FVector::ZeroVector;
	}

	UE_LOG(LogTemp, Warning, TEXT("bUp: %d, HitUp.Normal: %s"), bUp, *HitUp.Normal.ToString());
		

	float UpDot = FVector::DotProduct(HitUp.Normal, FVector::UpVector);

	if (UpDot < -0.1f) // -0.1보다 낮으면 실패
		return FVector::ZeroVector;
		



	//-------------------------------------------
	// 2) 윗면에서 아래로 Ray → 발딛기 위치
	//-------------------------------------------
	FVector DownStart = HitUp.ImpactPoint + FVector(0, 0, 20.f);
	FVector DownEnd = DownStart - FVector(0, 0, 130.f);

	FHitResult HitDown;
	bool bDown = GetWorld()->LineTraceSingleByChannel(
		HitDown,
		DownStart,
		DownEnd,
		ECC_GameTraceChannel4,
		Params
	);

	if (!bDown)
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace failed"));
		return FVector::ZeroVector;
	}
	DrawDebugCapsule(GetWorld(), SweepStart, HalfHeight * 0.3f, Radius * 0.8f, FQuat::Identity, FColor::Red, false, 5.f);
	DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Green, false, 5.f, 0, 2.f);

	UE_LOG(LogTemp, Warning, TEXT("bUp: %d, bDown: %d, UpDot: %f"), bUp, bDown, UpDot);

	FVector OutClimbPos = HitDown.ImpactPoint;  // 최종 올라갈 위치
	UE_LOG(LogTemp, Warning, TEXT("ClimbPos found: %s"), *HitDown.ImpactPoint.ToString());
	return OutClimbPos;
	
}

void AC_PlayerCharacter::startClimbUp()
{
	m_eState = E_CombatState::Climb;

	m_bCanClimbUp = false;
	m_bIsWallGrabbing = false;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	FVector AdjustedPos = m_vClimbLocation;
	AdjustedPos.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SetActorLocation(AdjustedPos);

	if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->playUpToClimb();
	}
	
}

void AC_PlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	m_nJumpCount = 0;
	m_bCanWallJump = true;
}

void AC_PlayerCharacter::handleWallGrabMovement(float fDelta)
{
	
}

bool AC_PlayerCharacter::isWallGrab() const
{
	return m_bIsWallGrabbing;
}

bool AC_PlayerCharacter::isCrouch() const
{
	return m_bIsCrouch;
}

bool AC_PlayerCharacter::isCanWallGrab() const
{
	return m_bCanWallGrab && !m_bIsWallGrabbing;
}

bool AC_PlayerCharacter::isWallGrabbing() const
{
	return m_bIsWallGrabbing;
}


void AC_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// 락온 기능
	setLockOn(DeltaTime);

	checkWallTrace();  // 무조건 실행



	/*if (m_bCanWallGrab && m_bJumpPressed)
	{
		if (checkClimbableSurface())
		{
			startClimbUp();
		}
	}*/
	
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
	}
}
