// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_OnActionFinished.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"

void UC_AnimNotify_OnActionFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ACharacter* pCharacter = Cast<ACharacter>(MeshComp->GetOwner());

	if (AC_PlayerCharacter* pPlayer = Cast<AC_PlayerCharacter>(pCharacter))
	{
		pPlayer->onActionFinished();
	}
}
