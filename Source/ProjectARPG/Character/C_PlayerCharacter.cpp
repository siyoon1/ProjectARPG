// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Animation/C_PlayerAnim.h"
#include "ProjectARPG/ActorComponents/C_GrappleComponent.h"

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

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;


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

	if (m_eState == E_CombatState::WallGrabbing)
	{
		wallGrabMove(vMoveDir);
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

	if (m_bIsWallGrabbing && m_eState == E_CombatState::WallGrabbing)
	{
		FVector ClimbPos = checkClimbableSurface();

		if (ClimbPos != FVector::ZeroVector)
		{
			setWallGrab(false);
			m_vClimbLocation = ClimbPos;
			startClimbUp();
			m_nJumpCount = 0;
		}
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
	UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance());

	if (!pAnim)
		return;

	if (m_eState != E_CombatState::Idle)
		return;

	FVector vInputDir = GetLastMovementInputVector().GetSafeNormal();

	const float fElapsedTime = sInst.GetElapsedTime();

	const float fHoldThreshold = 0.3f;

	if (fElapsedTime >= fHoldThreshold && GetLastMovementInputVector().IsNearlyZero())
	{
		if (m_eState != E_CombatState::Guard)
		{
			setCombatState(E_CombatState::Guard);
			pAnim->setIsGuarding(true);
			GetCharacterMovement()->MaxWalkSpeed = 400.f;
		}
	}
	
}

void AC_PlayerCharacter::crouch(const FInputActionValue& sValue)
{
	// 벽잡기 중이면 무조건 탈출 처리 먼저
	if (m_eState == E_CombatState::WallGrabbing)
	{
		setWallGrab(false);
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
	if (isCanWallGrab())
	{
		setWallGrab(true);
	}
}

void AC_PlayerCharacter::grapple(const FInputActionValue& sValue)
{
	if (m_pGrappleCom)
		m_pGrappleCom->tryStartGrapple();
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

	if (m_eState != E_CombatState::Idle)
		return;

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

	if (canExecute() && tryExcuteEnemy())
		return;

	if (!canAttack())
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
	case E_CombatState::WallGrabbing:
	case E_CombatState::Climb:
	case E_CombatState::Die:
		return false;

	case E_CombatState::Crouch:
		return false;


	default:
		return true;
	}
}

bool AC_PlayerCharacter::canExecute() const
{
	switch (m_eState)
	{
	case E_CombatState::Dodging:
	case E_CombatState::Executing:
	case E_CombatState::Climb:
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

USphereComponent* AC_PlayerCharacter::getExecutionSphere() const
{
	if (m_pExecutionDetectSphere)
		return m_pExecutionDetectSphere;

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


bool AC_PlayerCharacter::isLockOn() const
{
	return m_bIsLockOn;
}

void AC_PlayerCharacter::checkWallTrace()
{
	if (!isPlayerControlled())
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

	const float fMinGrabDistance = 80.f;

	if (bHit && Hit.Normal.Z < 0.5f &&
		FVector::Dist(Start, Hit.Location) <= fMinGrabDistance)
	{
		m_bCanWallGrab = true;

		// 새로 벽 붙을 때만 노멀 갱신
		if (!m_bIsWallGrabbing)
		{
			FVector N = Hit.Normal;
			N.Z = 0.f;
			m_vWallNormal = N.GetSafeNormal();
			m_vWallHitLocation = Hit.Location;
		}
	}
	else
	{
		m_bCanWallGrab = false;

		// 벽잡기 상태 유지 중이면 해제
		//if (m_bIsWallGrabbing)
			//setWallGrab(false);
	}

	DrawDebugLine(GetWorld(), Start, End, m_bCanWallGrab ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.f);
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

void AC_PlayerCharacter::setWallGrab(bool bEnable)
{

	if (bEnable)
	{
		m_bIsWallGrabbing = true;
		m_eState = E_CombatState::WallGrabbing;


		// 중력 제거 + 속도 제거
		
		GetCharacterMovement()->GravityScale = 0.f;
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->AirControl = 1.f;
		GetCharacterMovement()->Velocity = FVector::ZeroVector;

		bUseControllerRotationYaw = false;

		SetActorLocation(GetActorLocation() - (-m_vWallNormal * 10.f));
	}
	else
	{
		m_bIsWallGrabbing = false;

		m_eState = E_CombatState::Idle;

		
		GetCharacterMovement()->GravityScale = m_fDefaultGravity;
		GetCharacterMovement()->AirControl = m_fDefaultAirControl;

		bUseControllerRotationYaw = true;


		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

FVector AC_PlayerCharacter::checkClimbableSurface()
{
	if (!m_bIsWallGrabbing)
		return FVector::ZeroVector;

	float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	FVector ActorLoc = GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);



	// 벽 위 모서리 찾기
	FVector SweepStart = ActorLoc + FVector(0, 0, HalfHeight * 0.4f) - m_vWallNormal * 90.f;
	FVector SweepEnd = SweepStart + (-m_vWallNormal * 70.f);

	FHitResult FrontHit;
	bool bFront = GetWorld()->SweepSingleByChannel(
		FrontHit,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	if (!bFront)
	{
		UE_LOG(LogTemp, Warning, TEXT("Front sweep fail"));
		return FVector::ZeroVector;
	}

	//모서리 위에서 아래로 레이
	FVector TopStart = FrontHit.ImpactPoint + FVector(0, 0, 40.f);
	FVector TopEnd = TopStart - FVector(0, 0, 150.f);

	FHitResult DownHit;
	bool bDown = GetWorld()->LineTraceSingleByChannel
	(	DownHit,
		TopStart,
		TopEnd,
		ECC_Visibility,
		Params
	);

	if (!bDown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Down trace fail"));
		return FVector::ZeroVector;
	}

	float MaxClimbHeight = ActorLoc.Z +  HalfHeight + 2.f;


	if (DownHit.ImpactPoint.Z > MaxClimbHeight)
	{
		UE_LOG(LogTemp, Warning, TEXT("Climb too high, fail"));
		return FVector::ZeroVector;
	}


	// 최종 위치
	FVector FinalPos = DownHit.ImpactPoint;
	FinalPos.Z += HalfHeight;

	// Debug
	DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Yellow, false, 2.f, 0, 2.f);
	DrawDebugLine(GetWorld(), TopStart, TopEnd, FColor::Cyan, false, 2.f, 0, 2.f);
	DrawDebugSphere(GetWorld(), FinalPos, 10.f, 12, FColor::Green, false, 2.f);

	return FinalPos;
	
}

void AC_PlayerCharacter::startClimbUp()
{
	m_eState = E_CombatState::Climb;

	m_bCanClimbUp = false;
	m_bIsWallGrabbing = false;

	// ? 2) RootMotion 제어 위해 Flying
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);


	if (UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(GetMesh()->GetAnimInstance()))
	{

		pAnim->playUpToClimb();
		
	}
	
}

void AC_PlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	m_nJumpCount = 0;
}

void AC_PlayerCharacter::wallGrabMove(const FVector2D& MoveInput)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();

	// 좌우 입력만 사용 (W,S 무시)
	const float InputX = MoveInput.X;

	if (FMath::IsNearlyZero(InputX))
		return;

	// 중력 제거
	Move->GravityScale = 0.f;

	// 캐릭터 회전 고정
	bUseControllerRotationYaw = false;
	Move->bOrientRotationToMovement = false;

	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector WallRight = FVector::CrossProduct(m_vWallNormal, FVector::UpVector).GetSafeNormal();

	FVector SweepDir = WallRight * InputX;


	FHitResult Hit{};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(this);

	FVector vHead = GetActorLocation() + FVector(0, 0, HalfHeight * 0.7f);

	FVector vStart = vHead + SweepDir * 10.f + GetActorForwardVector() * 9.f;
	FVector vEnd = vHead + SweepDir * 40.f + GetActorForwardVector() * 9.f;

	const float fRadius = 30.f;


	bool bHit = 

	GetWorld()->SweepSingleByChannel
	(
		Hit,
		vStart,
		vEnd,
		FQuat::Identity,
		ECC_EngineTraceChannel4,
		FCollisionShape::MakeSphere(fRadius),
		Params
	);

	FVector vLast = GetActorLocation() + SweepDir * 1.5f;

	if (bHit)
	{
		SetActorLocation(vLast);
	}

	//DrawDebugSphere(GetWorld(), vStart, fRadius, 12, FColor::Green, false, 2.f);
	//DrawDebugSphere(GetWorld(), vEnd, fRadius, 12, FColor::Blue, false, 2.f);

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
	return m_bCanWallGrab && !m_bIsWallGrabbing && m_eState == E_CombatState::Idle;
}

bool AC_PlayerCharacter::isWallGrabbing() const
{
	return  m_eState == E_CombatState::WallGrabbing;
}

bool AC_PlayerCharacter::isPulling() const
{
	if (m_pGrappleCom)
		return m_pGrappleCom->isPulling();

	return false;
}

FVector AC_PlayerCharacter::getClimbLoc() const
{
	return m_vClimbLocation;
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


void AC_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// 락온 기능
	setLockOn(DeltaTime);

	checkWallTrace();  // 무조건 실행


	if(m_eState == E_CombatState::Climb)
	{
		FVector ActorLoc = GetActorLocation();

		// Tick 기준으로 RootMotion에 의해 이동한 X/Y값은 그대로 두고 Z만 Clamp
		FVector CurrentVelocity = GetCharacterMovement()->Velocity;

		// Z Clamp
		if (ActorLoc.Z > m_vClimbLocation.Z)
		{
			ActorLoc.Z = m_vClimbLocation.Z;

			SetActorLocation(ActorLoc, true);

			FVector NewVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);
			GetCharacterMovement()->Velocity = NewVelocity;
		}
	}

	
}

bool AC_PlayerCharacter::tryExcuteEnemy() const
{
	if (!m_pExecutionCom)
		return false;

	if (m_eState == E_CombatState::Climb ||
		m_eState == E_CombatState::Dodging ||
		m_eState == E_CombatState::Executing ||
		m_eState == E_CombatState::Guard)
		return false;

	return m_pExecutionCom->tryExecuteCurrentTarget();
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
