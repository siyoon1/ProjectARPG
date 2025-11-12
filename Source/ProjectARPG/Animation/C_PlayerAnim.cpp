// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerAnim.h"
#include "../Character/C_PlayerCharacter.h"

void UC_PlayerAnim::playDodgeMontage(E_Direction eDir)
{
	if (!m_pDodgeMontage)
		return;

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

void UC_PlayerAnim::playComboMontage(E_AttackType eType, int32 nComboIndex)
{
    switch(eType)
    {
    case E_AttackType::Normal:
        if (!m_pComboAttackMontages.IsValidIndex(nComboIndex - 1))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Combo Index: %d"), nComboIndex);
            return;
        }


        UAnimMontage* pMontageToPlay = m_pComboAttackMontages[nComboIndex - 1];

        if (!pMontageToPlay)
        {
            return;
        }

        Montage_Play(pMontageToPlay);
        break;
    }
    
    
}

void UC_PlayerAnim::playUpToClimb()
{
    if (!IsAnyMontagePlaying())
        Montage_Play(m_pUpToClimbMontage);
}

void UC_PlayerAnim::setIsGuarding(bool bNewGuard)
{
    m_bIsGuarding = bNewGuard;
}

bool UC_PlayerAnim::getIsGuarding() const
{
    return m_bIsGuarding;
}
