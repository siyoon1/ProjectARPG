// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AttackStart.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

void UC_AttackStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(MeshComp->GetOwner()))
	{
		if (pEnemy->getCurrentAttackData())
		{
			pEnemy->applyAttack(*pEnemy->getCurrentAttackData());
		}
	}
}
