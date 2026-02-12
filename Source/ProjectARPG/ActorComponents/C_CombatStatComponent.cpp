// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CombatStatComponent.h"
#include "ProjectARPG/Sturcts/FS_PostureStats.h"

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
	initStat();
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
		applyPostureDamage(PostureDamage, E_PostureBreakCause::Damage, nullptr);
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[Posture] Damage=%.1f Current=%.2f Delayed=%d"),
		PostureDamage,
		m_CurrentPosture,
		m_bRecoveryDelayed
	);
}

void UC_CombatStatComponent::applyPostureDamage(float PostureDamage, E_PostureBreakCause Cause, AActor* Instigator)
{
	if (PostureDamage <= 0.f || m_bPostureBroken)
		return;

	m_CurrentPosture = FMath::Clamp(m_CurrentPosture - PostureDamage, 0.f, m_MaxPosture);
	m_OnPostureChanged.Broadcast(m_CurrentPosture, m_MaxPosture);

	m_bRecoveryDelayed = true;
	m_RecoveryDelayTimer = m_RecoveryDelayTime;

	constexpr float PostureBreakThreshold = 3.f;

	if (m_CurrentPosture <= PostureBreakThreshold)
	{
		m_CurrentPosture = 0.f;

		m_LastBreakCause = Cause;
		m_LastBreaker = Instigator;

		breakPosture();
	}
}

void UC_CombatStatComponent::recoverHp(float Hp)
{
	m_CurrentHp = FMath::Clamp(m_CurrentHp + Hp, 0.f, m_MaxHp);
	m_OnHpChanged.Broadcast(m_CurrentHp, m_MaxHp);
}


void UC_CombatStatComponent::initStat()
{
	if (!m_StatTable || m_StatRowName.IsNone())
		return;

	FS_PostureStats* Row = m_StatTable->FindRow<FS_PostureStats>(m_StatRowName, TEXT("Stat Load"));

	if (!Row)
		return;

	m_MaxHp = Row->fMaxHp;
	m_MaxPosture = Row->fMaxPosture;
	m_PostureRecoveryRate = Row->fRecoveryRate;
	m_RecoveryDelayTime = Row->fRecoveryDelay;

	m_CurrentHp = m_MaxHp;
	m_CurrentPosture = m_MaxPosture;

}

void UC_CombatStatComponent::tickPostureRecovery(float DeltaTime)
{
	if (m_bPostureBroken)
		return;

	if (m_bRecoveryDelayed)
	{
		m_RecoveryDelayTimer -= DeltaTime;
		if (m_RecoveryDelayTimer <= 0.f)
		{
			m_bRecoveryDelayed = false;
		}
		return;
	}

	if (m_CurrentPosture <= KINDA_SMALL_NUMBER)
		return;

	if (m_CurrentPosture < m_MaxPosture)
	{
		m_CurrentPosture += m_PostureRecoveryRate * DeltaTime;
		m_CurrentPosture = FMath::Min(m_CurrentPosture, m_MaxPosture);

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

