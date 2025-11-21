// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyAnim.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "C_EnemyAnim.h"

void UC_EnemyAnim::playAttackMontage()
{
	if (!IsAnyMontagePlaying())
		Montage_Play(m_pAttackMontage);
}

void UC_EnemyAnim::AnimNotify_EndAttack()
{
	if (AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(TryGetPawnOwner()))
	{
		if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pOwner))
		{
			if (pEnemy->getCombatState() == E_CombatState::Attacking)
			{
				pEnemy->setCombatState(E_CombatState::Idle);
				UE_LOG(LogTemp, Error, TEXT("EndAttack!!!"));
			}
		}
	}
}
