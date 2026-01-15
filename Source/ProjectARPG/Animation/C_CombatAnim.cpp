// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CombatAnim.h"


void UC_CombatAnim::playHitMontage(E_Direction eDir)
{
    UAnimMontage* pMontageToPlay = nullptr;


    switch (eDir)
    {
    case E_Direction::Forward:
        pMontageToPlay = m_pHitLeftMontage;
        break;

    case E_Direction::Left:
        pMontageToPlay = m_pHitLeftMontage;
        break;

    case E_Direction::Right:
        pMontageToPlay = m_pHitRightMontage;
        break;

    case E_Direction::Backward:
        pMontageToPlay = m_pHitBackMontage;
        break;
    }


    if (!IsAnyMontagePlaying())
    {
        Montage_Stop(0.1f);
    }

    if (pMontageToPlay)
    {
        Montage_Play(pMontageToPlay, 1.0f);
    }
}

void UC_CombatAnim::playParryMontage(E_ParryDirection eDir)
{
    switch (eDir)
    {
    case E_ParryDirection::Left:
        Montage_Play(m_LeftParryMontage);
        break;

    case E_ParryDirection::Right:
        Montage_Play(m_RightParryMontage);
        break;

    case E_ParryDirection::Both:
    default:
        Montage_Play(m_DefaultParryMontage);
        break;
    }
}

