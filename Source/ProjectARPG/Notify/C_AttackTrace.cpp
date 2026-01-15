// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AttackTrace.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"



void UC_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (APawn* pPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (AC_CombatCharacter* pChar = Cast<AC_CombatCharacter>(pPawn))
		{

			if (m_HitParryDir != E_ParryDirection::None)
			{
				pChar->setRuntimeParryDir(m_HitParryDir);
			}

			pChar->startAttackTrace();
		}
	}

}

void UC_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (APawn* pPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (AC_CombatCharacter* pChar = Cast<AC_CombatCharacter>(pPawn))
		{
			pChar->performAttackTrace();
		}
	}
}

void UC_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (APawn* pPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		if (AC_CombatCharacter* pChar = Cast<AC_CombatCharacter>(pPawn))
		{
			pChar->stopAttackTrace();

			pChar->setRuntimeParryDir(E_ParryDirection::None);
		}
	}
}
