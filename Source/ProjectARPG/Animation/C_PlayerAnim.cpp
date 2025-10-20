// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerAnim.h"
#include "../Character/C_PlayerCharacter.h"

void UC_PlayerAnim::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OnMontageEnded.AddDynamic(this, &UC_PlayerAnim::onMontageEnded);

    
}

void UC_PlayerAnim::NativeUninitializeAnimation()
{
    Super::NativeUninitializeAnimation();

    OnMontageEnded.RemoveDynamic(this, &UC_PlayerAnim::onMontageEnded);
}

void UC_PlayerAnim::onMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/*if (Montage != m_pComboAttackMontage)
		return;*/
	if (AC_PlayerCharacter* pPlayer = Cast<AC_PlayerCharacter>(TryGetPawnOwner()))
	{
		pPlayer->resetComboState();
		pPlayer->enableComboRestart();

	}
    
}

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

void UC_PlayerAnim::playComboMontage(int32 nComboIndex)
{

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
        
    if (!Montage_IsPlaying(pMontageToPlay))
        Montage_Play(pMontageToPlay);
}
