// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_ExecutionFinished.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

void UC_AnimNotify_ExecutionFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
		return;

	AC_EnemyCharacter* Enemy =
		Cast<AC_EnemyCharacter>(Owner);

	if (!Enemy)
		return;

	Enemy->onExecutionFinished(nullptr);
}
