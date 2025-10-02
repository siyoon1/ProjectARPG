// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerCameraManager.h"

void AC_PlayerCameraManager::startSprintEffect()
{
	m_PostProcessSettings.bOverride_MotionBlurAmount = true;
	m_PostProcessSettings.MotionBlurAmount = 0.7f;

	SetFOV(DefaultFOV + 10.f);


}

void AC_PlayerCameraManager::stopSprintEffect()
{
	
	m_PostProcessSettings.MotionBlurAmount = 0.f;

	m_PostProcessSettings.bOverride_MotionBlurAmount = false;

	SetFOV(DefaultFOV);
}
