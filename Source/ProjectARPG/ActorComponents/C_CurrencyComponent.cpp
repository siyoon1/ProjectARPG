// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CurrencyComponent.h"

UC_CurrencyComponent::UC_CurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UC_CurrencyComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void UC_CurrencyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

int32 UC_CurrencyComponent::getGold() const
{
	return m_Gold;
}

void UC_CurrencyComponent::addGold(int32 Amount)
{
	if (Amount <= 0)
		return;

	m_Gold += Amount;
	m_OnCurrencyChanged.Broadcast();
}

bool UC_CurrencyComponent::spendGold(int32 Amount)
{
	if (Amount <= 0)
		return false;

	if (m_Gold < Amount)
		return false;

	m_Gold -= Amount;
	m_OnCurrencyChanged.Broadcast();

	return true;
}

