// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_onExecutionFinished.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"

void UC_AnimNotify_onExecutionFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AC_PlayerCharacter* Player = Cast<AC_PlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->onExecutionFinished();
	}
}
