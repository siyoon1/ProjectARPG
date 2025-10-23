// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyCharacter.h"
#include "Components/WidgetComponent.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"

void AC_EnemyCharacter::showHpBar(bool bShow)
{
	m_wHpBarCom = GetComponentByClass<UWidgetComponent>();

	if (m_wHpBarCom)
		m_wHpBarCom->SetVisibility(bShow);
}

void AC_EnemyCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage)
{
	Super::takeDamage_Implementation(fDamage, fPostureDamage);

	if (m_fCurrentPosture >= m_fMaxPosture && !m_bCanbeExcuted)
	{
		m_fCurrentPosture = m_fMaxPosture;
		m_bCanbeExcuted = true;

		if (UC_ExecutionComponent* pExcutionCom = FindComponentByClass<UC_ExecutionComponent>())
		{
			pExcutionCom->onBecomeExcutable(this);
		}


	}


}
