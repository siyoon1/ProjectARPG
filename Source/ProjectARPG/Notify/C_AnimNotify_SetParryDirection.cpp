// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_SetParryDirection.h"
#include "ProjectARPG/ActorComponents/C_AttackComponent.h"

void UC_AnimNotify_SetParryDirection::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	if (AC_CombatCharacter* Owner =
		Cast<AC_CombatCharacter>(MeshComp->GetOwner()))
	{
		if (UC_AttackComponent* AttackComp =
			Owner->FindComponentByClass<UC_AttackComponent>())
		{
			AttackComp->overrideParryDirection(m_NewDirection);
		}
	}
}
