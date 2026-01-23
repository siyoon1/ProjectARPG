// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_AIActionFinished.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

void UC_AnimNotify_AIActionFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(MeshComp->GetOwner()))
	{
		Enemy->onActionCooldownFinished();
	}
}
