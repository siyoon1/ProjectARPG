// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyAnim.h"
#include "C_EnemyAnim.h"

void UC_EnemyAnim::playAttackByType(E_EnemyAttackType eType)
{
	switch (eType)
	{
	case E_EnemyAttackType::Light:
		Montage_Play(m_pLightAttackMontage);
		break;

	case E_EnemyAttackType::Heavy:
		Montage_Play(m_pHeavyAttackMontage);
		break;

	case E_EnemyAttackType::Thrust:
		Montage_Play(m_pThrustAttackMontage);
		break;
	}
}

void UC_EnemyAnim::AnimNotify_EndAttack()
{
	if (AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(TryGetPawnOwner()))
	{
		if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pOwner))
		{
			pEnemy->endAttack();
		}
	}
}
