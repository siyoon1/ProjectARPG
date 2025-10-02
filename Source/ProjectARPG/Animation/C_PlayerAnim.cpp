// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerAnim.h"

void UC_PlayerAnim::playDodgeMontage(E_Direction eDir)
{
	if (!m_pDodgeMontage)
		return;

	Montage_Play(m_pDodgeMontage);
}

void UC_PlayerAnim::playSprintStartMontage()
{
	if (!IsAnyMontagePlaying())
		Montage_Play(m_pSprintStartMontage);
	
		
	
}
