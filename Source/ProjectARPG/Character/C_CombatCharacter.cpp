// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CombatCharacter.h"
#include "ProjectARPG/Sturcts/FS_PostureStats.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AC_CombatCharacter::AC_CombatCharacter()
{

}

void AC_CombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void AC_CombatCharacter::setHp(float fHp)
{
	m_fCurrnetHp = fHp;
}

float AC_CombatCharacter::getHp() const
{
	return m_fCurrnetHp;
}

float AC_CombatCharacter::getMaxHp() const
{
	return m_fMaxHp;
}

float AC_CombatCharacter::getPosture() const
{
	return m_fCurrentPosture;
}

float AC_CombatCharacter::getMaxPosture() const
{

	return m_fMaxPosture;
}

void AC_CombatCharacter::startAttackTrace()
{
	if (m_bIsTracing)
		return;

	m_bIsTracing = true;
	m_HitActors.Empty();

	if (m_pTraceStart)
		m_vLastTraceStart = m_pTraceStart->GetComponentLocation();
	if (m_pTraceEnd)
		m_vLastTraceEnd = m_pTraceEnd->GetComponentLocation();
}

void AC_CombatCharacter::stopAttackTrace()
{
	m_bIsTracing = false;
	m_HitActors.Empty();
}

void AC_CombatCharacter::performAttackTrace()
{
	if (!m_bIsTracing)
		return;

	if (!m_pTraceStart || !m_pTraceEnd)
		return;

	FVector vPrevStart = m_vLastTraceStart;
	FVector vPrevEnd = m_vLastTraceEnd;

	FVector vCurStart = m_pTraceStart->GetComponentLocation();
	FVector vCurEnd = m_pTraceEnd->GetComponentLocation();

	m_vLastTraceStart = vCurStart;
	m_vLastTraceEnd = vCurEnd;


	TArray<FHitResult> HitRes{};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitRes,
		vPrevStart,
		vCurEnd,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(m_fTraceRadius),
		Params
	);


#if WITH_EDITOR
	
	DrawDebugCapsule(GetWorld(), (vPrevStart + vCurEnd) * 0.5f, FVector::Distance(vPrevStart, vCurEnd) * 0.5f,
		m_fTraceRadius, FQuat::Identity, bHit ? FColor::Green : FColor::Red, false, 0.05f);
#endif 

	if (bHit)
	{
		for (const FHitResult& Hit : HitRes)
		{
			AActor* pHitActor = Hit.GetActor();

			if (!pHitActor)
				continue;

			if (pHitActor == this)
				continue;

			if (m_HitActors.Contains(pHitActor))
				continue;


			if (pHitActor->GetClass()->ImplementsInterface(UC_CombatInterface::StaticClass()))
			{
				float fFinalDamage = m_fAttackDamage;
				float fFinalPostureDamage = m_fPostureDamage;

				switch (m_eAttackType)
				{
				case E_AttackType::Normal:
					// 기본값 그대로
					break;

				case E_AttackType::Air:
					fFinalDamage *= 0.8f;
					fFinalPostureDamage *= 1.0f;
					break;
				}
				IC_CombatInterface::Execute_takeDamage(pHitActor, fFinalDamage, fFinalPostureDamage);
			}

			m_HitActors.Add(pHitActor);
		}
	}

}

void AC_CombatCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage)
{
	if (m_fCurrnetHp > 0)
		m_fCurrnetHp = FMath::Clamp(m_fCurrnetHp - fDamage, 0.f, m_fMaxHp);

	if (m_fCurrentPosture > 0)
		m_fCurrentPosture = FMath::Clamp(m_fCurrentPosture - fPostureDamage, 0.f, m_fMaxPosture);

	m_OnHpChanged.Broadcast(m_fCurrnetHp, m_fMaxHp);
	UE_LOG(LogTemp, Warning, TEXT("TakeDamage: HP %.1f / %.1f"), m_fCurrnetHp, m_fMaxHp);

	if (m_sPostureStats)
		m_OnPostureChanged.Broadcast(m_fCurrentPosture, m_fMaxPosture);


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
			m_fMaxPosture = m_sPostureStats->fMaxPosture;
			m_fCurrentPosture = m_fMaxPosture;
			m_fRecoveryRate = m_sPostureStats->fRecoveryRate;
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Posture Stats Row '%s' not found!"), *m_sPostureRowName.ToString());
		}
	}

	m_fCurrnetHp = m_fMaxHp;

	if (!m_pTraceStart)
	{
		m_pTraceStart = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("TraceStart")));
	}

	if (!m_pTraceEnd)
	{
		m_pTraceEnd = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("TraceEnd")));
	}


	if (m_pTraceStart && m_pTraceEnd)
	{

		m_vLastTraceStart = m_pTraceStart->GetComponentLocation();
		m_vLastTraceEnd = m_pTraceEnd->GetComponentLocation();
	
	}

	m_pExecutionCom = GetComponentByClass<UC_ExecutionComponent>();
	
	
}
