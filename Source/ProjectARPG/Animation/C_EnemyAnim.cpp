// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyAnim.h"
#include "C_EnemyAnim.h"

void UC_EnemyAnim::playStepBackMontage()
{
	if (!m_StepBackMontage)
		return;

	Montage_Play(m_StepBackMontage);
}

void UC_EnemyAnim::playAttackMontage(UAnimMontage* pAttackMontage)
{
	if (!pAttackMontage)
		return;

	Montage_Play(pAttackMontage);
}
