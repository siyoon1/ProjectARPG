// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Animation/C_PlayerAnim.h"
#include "../Camera/C_PlayerCameraManager.h"

AC_PlayerCharacter::AC_PlayerCharacter()
{
	m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_pSpringArm->SetupAttachment(RootComponent);
	m_pSpringArm->TargetArmLength = 500.f;
	m_pSpringArm->bUsePawnControlRotation = true;

	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_pCamera->SetupAttachment(m_pSpringArm);
	m_pCamera->bUsePawnControlRotation = false;

}

void AC_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = 800.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	if (APlayerController* pPlayerCon = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* pSubSystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pPlayerCon->GetLocalPlayer()))
		{
			pSubSystem->AddMappingContext(m_pMappingContext, 0);
		}
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

	
	if (sInst.GetTriggerEvent() != ETriggerEvent::Triggered)
		return;	

	const float fElapsedTime = sInst.GetElapsedTime();


	const float fHoldThreshold = 0.3f;

	if (fElapsedTime >= fHoldThreshold)
	{
		// 대시 실행
		if (m_eState != E_PlayerActionState::Sprinting)
		{
			pAnim->playSprintStartMontage();
			m_eState = E_PlayerActionState::Sprinting;
			if (APlayerController* pPlayerCon = Cast<APlayerController>(GetController()))
			{
				if (AC_PlayerCameraManager* pCameraMgr = Cast<AC_PlayerCameraManager>(pPlayerCon->PlayerCameraManager))
				{
					pCameraMgr->startSprintEffect();
				}
			}
			
		}	
		GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	}
	else
	{
		// 회피 실행

		pAnim->playDodgeMontage(E_Direction::Backward);
		
	}

	GetCharacterMovement()->MaxWalkSpeed = 800.f;
	
}

void AC_PlayerCharacter::sprintReleased(const FInputActionInstance& sInst)
{
	if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying() && m_eState == E_PlayerActionState::Sprinting)
		GetMesh()->GetAnimInstance()->Montage_Stop(0.1f);

	m_eState = E_PlayerActionState::Idle;
	GetCharacterMovement()->MaxWalkSpeed = 800.f;
	if (APlayerController* pPlayerCon = Cast<APlayerController>(GetController()))
	{
		if (AC_PlayerCameraManager* pCameraMgr = Cast<AC_PlayerCameraManager>(pPlayerCon->PlayerCameraManager))
		{
			pCameraMgr->stopSprintEffect();
		}
	}
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
	}
}
