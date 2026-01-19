// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CombatStatComponent.h"

// Sets default values for this component's properties
UC_CombatStatComponent::UC_CombatStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UC_CombatStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	m_CurrentHp = m_MaxHp;
	m_CurrentPosture = m_MaxPosture;
}


// Called every frame
void UC_CombatStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (m_bPostureBroken)
		return;

	tickPostureRecovery(DeltaTime);
}

void UC_CombatStatComponent::applyDamage(float HpDamage, float PostureDamage)
{
	if (HpDamage > 0.f)
	{
		m_CurrentHp = FMath::Clamp(m_CurrentHp - HpDamage, 0.f, m_MaxHp);
		m_OnHpChanged.Broadcast(m_CurrentHp, m_MaxHp);
	}

	if (PostureDamage > 0.f)
	{
		m_CurrentPosture = FMath::Clamp(m_CurrentPosture - PostureDamage, 0.f, m_MaxPosture);
		m_OnPostureChanged.Broadcast(m_CurrentPosture, m_MaxPosture);

		m_bRecoveryDelayed = true;
		m_RecoveryDelayTimer = m_RecoveryDelayTime;

		if (m_CurrentPosture <= 0.f)
		{
			breakPosture();
		}
	}
}


void UC_CombatStatComponent::tickPostureRecovery(float DeltaTime)
{
	if (m_bRecoveryDelayed)
	{
		m_RecoveryDelayTimer -= DeltaTime;
		if (m_RecoveryDelayTimer <= 0.f)
		{
			m_bRecoveryDelayed = false;
		}
		return;
	}

	if (m_CurrentPosture < m_MaxPosture)
	{
		m_CurrentPosture = FMath::Clamp(
			m_CurrentPosture + m_PostureRecoveryRate * DeltaTime,
			0.f,
			m_MaxPosture
		);

		m_OnPostureChanged.Broadcast(m_CurrentPosture, m_MaxPosture);
	}
}

void UC_CombatStatComponent::breakPosture()
{
	if (m_bPostureBroken)
		return;

	m_bPostureBroken = true;
	m_OnPostureBroken.Broadcast();
}

