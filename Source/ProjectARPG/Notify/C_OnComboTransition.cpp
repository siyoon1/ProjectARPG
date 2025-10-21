// Fill out your copyright notice in the Description page of Project Settings.


#include "C_OnComboTransition.h"
#include "../Character/C_PlayerCharacter.h"

void UC_OnComboTransition::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	ACharacter* pCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (AC_PlayerCharacter* pPlayer = Cast<AC_PlayerCharacter>(pCharacter))
	{
		pPlayer->onComboTransition();
	}
}
