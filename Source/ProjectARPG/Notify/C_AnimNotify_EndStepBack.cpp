// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_EndStepBack.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

void UC_AnimNotify_EndStepBack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(MeshComp->GetOwner()))
	{
		pEnemy->endStepBack();
	}
}
