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

void AC_CombatCharacter::setHp(float fHp)
{
	m_fCurrnetHp = fHp;
}

float AC_CombatCharacter::getHp() const
{
	return m_fCurrnetHp;
}

float AC_CombatCharacter::getPosture() const
{
	return m_fCurrentPosture;
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
				IC_CombatInterface::Execute_takeDamage(pHitActor, m_fAttackDamage, m_fPostureDamage);
			}

			m_HitActors.Add(pHitActor);
		}
	}

}

void AC_CombatCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage)
{
	if (m_fCurrnetHp > 0)
		m_fCurrnetHp = FMath::Clamp(m_fCurrnetHp - fDamage, 0.f, m_fMaxHp);

	m_fCurrentPosture -= fPostureDamage;

	m_OnHpChanged.Broadcast(m_fCurrnetHp, m_fMaxHp);
	UE_LOG(LogTemp, Warning, TEXT("TakeDamage: HP %.1f / %.1f"), m_fCurrnetHp, m_fMaxHp);


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
	
	
}
