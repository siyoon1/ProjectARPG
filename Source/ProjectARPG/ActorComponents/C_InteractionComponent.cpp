// Fill out your copyright notice in the Description page of Project Settings.


#include "C_InteractionComponent.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG//Character/C_NPCCharacter.h"
#include "Components/WidgetComponent.h"



// Sets default values for this component's properties
UC_InteractionComponent::UC_InteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	m_Owner = Cast<AC_NPCCharacter>(GetOwner());

}


// Called every frame
void UC_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_InteractionComponent::showIneractUI()
{
	if (m_Owner)
	{
		m_Owner->getInteractWidgetComp()->SetVisibility(true);
	}
}

void UC_InteractionComponent::hideInteractUI()
{
	if (m_Owner)
	{
		m_Owner->getInteractWidgetComp()->SetVisibility(false);
	}
}

void UC_InteractionComponent::onPlayerEnter(AC_PlayerCharacter* Player)
{
	m_CachedPlayer = Player;
	showIneractUI();

}

void UC_InteractionComponent::onPlayerExit()
{
	m_CachedPlayer = nullptr;
	hideInteractUI();
}

bool UC_InteractionComponent::canInteract() const
{
	return m_CachedPlayer != nullptr;
}


