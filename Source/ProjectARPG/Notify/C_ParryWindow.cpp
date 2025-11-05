// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ParryWindow.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"

void UC_ParryWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp->GetOwner())
		return;

	if (UC_ParryComponent* pParryCom = MeshComp->GetOwner()->GetComponentByClass<UC_ParryComponent>())
	{
		pParryCom->startParryWindow(m_fCanTime);
		UE_LOG(LogTemp, Warning, TEXT("ParryWindow Notify Triggered on %s"), *MeshComp->GetOwner()->GetName());
	}


	
}
