// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ParryComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"

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
	if (!m_bCanParry)
	{
		UE_LOG(LogTemp, Warning, TEXT("Parry failed: not in window"));
		return false;
	}

	AActor* pOwner = GetOwner(); // 패링 주체

	m_OnSuccessParry.Broadcast(pOwner, pAttacker);

	UE_LOG(LogTemp, Warning, TEXT("Parry SUCCESS: %s (ParryOwner) vs %s (Attacker)"),
		*pOwner->GetName(),
		*pAttacker->GetName());

	return true;
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

