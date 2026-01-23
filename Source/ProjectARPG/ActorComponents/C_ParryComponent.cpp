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

void UC_ParryComponent::startParryWindow(float Duration)
{
	if (!GetWorld())
		return;

	m_ParryContext.bWindowOpen = true;
	m_ParryContext.EndTime = GetWorld()->GetTimeSeconds() + Duration;


	GetWorld()->GetTimerManager().ClearTimer(m_ParryTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(m_ParryTimerHandle, this, &UC_ParryComponent::endParryWindow,
		Duration, false);
}

FS_ParryResult UC_ParryComponent::evaluateParry(const FS_AttackData& AttackData, AActor* Attacker) const
{
	FS_ParryResult Result;

	// 패링 윈도우 열려있는지?
	if (!isParryWindowOpen())
	{
		Result.Result = E_ParryResult::Failed;
		return Result;
	}

	// 이 공격은 패링이 가능한지?
	if (!AttackData.Combat.bCanParry)
	{
		Result.Result = E_ParryResult::Guarded;
		return Result;
	}

	Result.Result = E_ParryResult::Parried;
	Result.Direction = AttackData.Combat.ParryDirection;

	Result.PostureDamageToAttacker = AttackData.Combat.PostureDamage * 1.5f;
	Result.PostureDamageToDefender = AttackData.Combat.PostureDamage * 0.2f;

	return Result;
}

bool UC_ParryComponent::isParryWindowOpen() const
{
	if (!m_ParryContext.bWindowOpen)
		return false;

	return GetWorld() &&
		GetWorld()->GetTimeSeconds() <= m_ParryContext.EndTime;
}

void UC_ParryComponent::endParryWindow()
{
	if (!m_ParryContext.bWindowOpen)
		return;

	m_ParryContext.bWindowOpen = false;

	m_OnParryWindowEnded.Broadcast();

	
	UE_LOG(LogTemp, Warning, TEXT("%s ParryWindow End"), *GetOwner()->GetName());

	
}


