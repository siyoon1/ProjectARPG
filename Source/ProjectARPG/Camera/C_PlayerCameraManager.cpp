// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCameraManager.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"

AC_PlayerCameraManager::AC_PlayerCameraManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AC_PlayerCameraManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float fTargetFOV = DefaultFOV;

	if (m_bIsExecuting)
	{
		fTargetFOV = DefaultFOV - 20.f;
		
	}
	if (m_bIsParrying)
	{
		fTargetFOV = DefaultFOV - 10.f;
	}
	else if (m_bIsSprinting)
	{
		fTargetFOV = DefaultFOV + 10.f;
	}
	else if (m_bIsReturningFOV)
	{
		fTargetFOV = DefaultFOV;
	}
	else
		PrimaryActorTick.bCanEverTick = false;

	float CurrentFOV = GetFOVAngle();
	float NewFOV = FMath::FInterpTo(CurrentFOV, fTargetFOV, DeltaSeconds, 8.0f);
	SetFOV(NewFOV);

	if (m_bIsReturningFOV && FMath::IsNearlyEqual(NewFOV, DefaultFOV, 0.01f))
	{
		m_bIsReturningFOV = false;
	}
}

void AC_PlayerCameraManager::startSprintEffect()
{
	m_PostProcessSettings.bOverride_MotionBlurAmount = true;
	m_PostProcessSettings.MotionBlurAmount = 0.7f;


	m_bIsSprinting = true;

}

void AC_PlayerCameraManager::stopSprintEffect()
{
	
	m_PostProcessSettings.MotionBlurAmount = 0.f;

	m_PostProcessSettings.bOverride_MotionBlurAmount = false;

	m_bIsSprinting = false;
}

void AC_PlayerCameraManager::executionEffect(float fLength)
{
	m_bIsExecuting = true;
	PrimaryActorTick.bCanEverTick = true;

	m_PostProcessSettings.bOverride_MotionBlurAmount = true;
	m_PostProcessSettings.MotionBlurAmount = 0.9f;

	
	if (UWorld* pWorld = GetWorld())
	{
		pWorld->GetWorldSettings()->SetTimeDilation(0.8f);

		pWorld->GetTimerManager().SetTimer(m_TimerHandle_Reset, [this]()
			{
				m_bIsReturningFOV = true;

				m_PostProcessSettings.MotionBlurAmount = 0.f;
				m_PostProcessSettings.bOverride_MotionBlurAmount = false;

				m_PostProcessSettings.bOverride_ColorSaturation = false;
				m_PostProcessSettings.bOverride_BloomIntensity = false;

				if (UWorld* pWorldInner = GetWorld())
				{
					pWorldInner->GetWorldSettings()->SetTimeDilation(1.0f);
				}

				m_bIsExecuting = false;

			}, fLength, false);
	}
		
	

	
	
}

void AC_PlayerCameraManager::parryEffect(float fLength)
{
	m_bIsParrying = true;
	PrimaryActorTick.bCanEverTick = true;

	m_PostProcessSettings.bOverride_MotionBlurAmount = true;
	m_PostProcessSettings.MotionBlurAmount = 0.9f;


	if (UWorld* pWorld = GetWorld())
	{
		pWorld->GetWorldSettings()->SetTimeDilation(0.7f);

		pWorld->GetTimerManager().SetTimer(m_TimerHandle_Reset, [this]()
			{
				m_bIsReturningFOV = true;

				m_PostProcessSettings.MotionBlurAmount = 0.f;
				m_PostProcessSettings.bOverride_MotionBlurAmount = false;

				m_PostProcessSettings.bOverride_ColorSaturation = false;
				m_PostProcessSettings.bOverride_BloomIntensity = false;

				if (UWorld* pWorldInner = GetWorld())
				{
					pWorldInner->GetWorldSettings()->SetTimeDilation(1.0f);
				}

				m_bIsExecuting = false;

			}, fLength, false);
	}
}

