// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerAnim.h"

void UC_PlayerAnim::AnimNotify_DodgeEnd()
{
    UE_LOG(LogTemp, Log, TEXT("AnimNotify_DodgeEnd called"));
    m_bIsDodging = false;
}

void UC_PlayerAnim::playDodgeMontage(E_Direction eDir)
{
	if (!m_pDodgeMontage)
		return;

    if (m_bIsDodging)
        return;

    m_bIsDodging = true;

    FName SectionName;
    switch (eDir)
    {
    case E_Direction::Forward:
        SectionName = "Forward";
        break;
    case E_Direction::Backward:
        SectionName = "Back";    
        break;
    case E_Direction::Left:    
        SectionName = "Left";    
        break;
    case E_Direction::Right:   
        SectionName = "Right";   
        break;
    }


	
	Montage_Play(m_pDodgeMontage);
	Montage_JumpToSection(SectionName, m_pDodgeMontage);
 
   
}

void UC_PlayerAnim::playSprintStartMontage()
{
	if (!IsAnyMontagePlaying())
		Montage_Play(m_pSprintStartMontage);
	
		
	
}

void UC_PlayerAnim::playAttackMontage()
{
	if (!IsAnyMontagePlaying())
		Montage_Play(m_pComboAttackMontage);
}
