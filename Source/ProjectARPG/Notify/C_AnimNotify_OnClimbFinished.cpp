// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimNotify_OnClimbFinished.h"
#include "ProjectARPG/ActorComponents/C_MoveActionComponent.h"

void UC_AnimNotify_OnClimbFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UC_MoveActionComponent* MoveActionComp = Cast<UC_MoveActionComponent>(MeshComp->GetOwner()->GetComponentByClass<UC_MoveActionComponent>());

	if (!MoveActionComp)
		return;

	MoveActionComp->onClimbFinished();
}
