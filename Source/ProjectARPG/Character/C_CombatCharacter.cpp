// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CombatCharacter.h"
#include "ProjectARPG/Sturcts/FS_PostureStats.h"

AC_CombatCharacter::AC_CombatCharacter()
{
	
}

void AC_CombatCharacter::setHp(float fHp)
{
	m_fCurrnetHp = fHp;
}

float AC_CombatCharacter::getHp() const
{
	return m_fCurrnetHp;
}

void AC_CombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (m_pPostureStatsTable)
	{
		FS_PostureStats* pLoadedStats = m_pPostureStatsTable->FindRow<FS_PostureStats>(m_sPostureRowName, TEXT("Posture Data Load"));
		if (pLoadedStats)
		{
			m_sPostureStats = pLoadedStats;
			m_fCurrentPosture = m_sPostureStats->fMaxPosture;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Posture Stats Row '%s' not found!"), *m_sPostureRowName.ToString());
		}
	}

	m_fCurrnetHp = m_fMaxHp;
	
}
