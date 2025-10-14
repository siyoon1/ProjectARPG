// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SprintEnd.h"
#include "../Character/C_PlayerCharacter.h"

void UC_SprintEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ACharacter* pCharacter = Cast<ACharacter>(MeshComp->GetOwner());

	if (AC_PlayerCharacter* pPlayer = Cast<AC_PlayerCharacter>(pCharacter))
	{
		pPlayer->setPlayerActionState(E_PlayerActionState::Idle);
	}
}
