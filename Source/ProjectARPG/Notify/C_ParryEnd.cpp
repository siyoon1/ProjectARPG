// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ParryEnd.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

void UC_ParryEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(MeshComp->GetOwner()))
	{
		pEnemy->onParryFinished();
	}
}
