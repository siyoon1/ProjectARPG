// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AC_PlayerCharacter::AC_PlayerCharacter()
{
	m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_pSpringArm->SetupAttachment(RootComponent);
	m_pSpringArm->TargetArmLength = 800.f;
	m_pSpringArm->bUsePawnControlRotation = true;

	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_pCamera->SetupAttachment(m_pSpringArm);
	m_pCamera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}
