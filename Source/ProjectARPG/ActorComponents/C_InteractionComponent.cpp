// Fill out your copyright notice in the Description page of Project Settings.


#include "C_InteractionComponent.h"
#include "ProjectARPG/Character/C_BaseCharacter.h"
#include "Components/WidgetComponent.h"
#include "ProjectARPG/Interface/C_Interactable.h"



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

}


// Called every frame
void UC_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_InteractionComponent::tryInteract()
{
	if (!m_CurrentTarget)
		return false;

	IC_Interactable::Execute_interact(m_CurrentTarget, GetOwner());
	return true;
}

void UC_InteractionComponent::showUI(AActor* Target)
{
	if (!Target) return;

	if (UWidgetComponent* Widget = Target->FindComponentByClass<UWidgetComponent>())
	{
		Widget->SetVisibility(true);
	}
}

void UC_InteractionComponent::hideUI(AActor* Target)
{
	if (!Target) return;

	if (UWidgetComponent* Widget = Target->FindComponentByClass<UWidgetComponent>())
	{
		Widget->SetVisibility(false);
	}
}

void UC_InteractionComponent::updateCurrentTarget()
{
	AActor* Best = nullptr;
	float fBestDist = FLT_MAX;

	for (AActor* act : m_InteractableList)
	{
		float fDist = FVector::Dist(GetOwner()->GetActorLocation(), act->GetActorLocation());

		if (fDist < fBestDist)
		{
			fBestDist = fDist;
			Best = act;
		}

		if (Best != m_CurrentTarget)
		{
			hideUI(m_CurrentTarget);
			m_CurrentTarget = Best;
			showUI(m_CurrentTarget);
		}
	}
}

void UC_InteractionComponent::registerInteractable(AActor* Actor)
{
	if (!Actor || !Actor->Implements<UC_Interactable>())
		return;

	m_InteractableList.AddUnique(Actor);
	updateCurrentTarget();
}

void UC_InteractionComponent::unregisterInteractable(AActor* Actor)
{
	m_InteractableList.Remove(Actor);

	if (m_CurrentTarget == Actor)
	{
		hideUI(m_CurrentTarget);
		m_CurrentTarget = nullptr;
		updateCurrentTarget();
	}
}


