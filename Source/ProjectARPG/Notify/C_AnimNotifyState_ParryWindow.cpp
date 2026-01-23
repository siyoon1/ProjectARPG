// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotifyState_ParryWindow.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"

void UC_AnimNotifyState_ParryWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(MeshComp->GetOwner()))
	{
		if (Enemy->getParryComponent() && Enemy->getCurrentAttackData())
		{
			
		}
	}
}

void UC_AnimNotifyState_ParryWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(MeshComp->GetOwner()))
	{
		if (Enemy->getParryComponent())
		{
			
		}
	}
}
