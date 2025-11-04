// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ParryComponent.h"

// Sets default values for this component's properties
UC_ParryComponent::UC_ParryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_ParryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UC_ParryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_ParryComponent::tryParry(AActor* pAttacker)
{
	UE_LOG(LogTemp, Warning, TEXT("TryParry on %s | CanParry = %s"),
		*GetOwner()->GetName(),
		m_bCanParry ? TEXT("TRUE") : TEXT("FALSE"));

	if (!m_bCanParry)
		return false;

	if (m_bCanParry)
	{
		m_OnSuccessParry.Broadcast(GetOwner(), pAttacker);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Parry Success! Owner: %s, Attacker: %s"),
		*GetOwner()->GetName(), *pAttacker->GetName());

	return false;
}

void UC_ParryComponent::startParryWindow(float fCanTime)
{
	m_bCanParry = true;

	GetWorld()->GetTimerManager().ClearTimer(m_ParryTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(m_ParryTimerHandle, this, &UC_ParryComponent::endParryWindow,
		fCanTime, false);
	UE_LOG(LogTemp, Warning, TEXT("%s ParryWindow Start (%.2fs)"), *GetOwner()->GetName(), fCanTime);

}

void UC_ParryComponent::endParryWindow()
{
	m_bCanParry = false;
	UE_LOG(LogTemp, Warning, TEXT("%s ParryWindow End"), *GetOwner()->GetName());
}

