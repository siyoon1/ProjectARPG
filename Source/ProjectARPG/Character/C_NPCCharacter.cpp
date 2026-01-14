// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NPCCharacter.h"
#include "Components/WidgetComponent.h"

void AC_NPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_InteractWidgetComp = GetComponentByClass<UWidgetComponent>();

	if (m_InteractWidgetComp)
		m_InteractWidgetComp->SetVisibility(false);
}

void AC_NPCCharacter::interact_Implementation(AActor* Interator)
{

}

UWidgetComponent* AC_NPCCharacter::getInteractWidgetComp() const
{
	return m_InteractWidgetComp;
}

const TArray<FText>& AC_NPCCharacter::getDialogueLine() const
{
	return m_DialogueLine;
}
