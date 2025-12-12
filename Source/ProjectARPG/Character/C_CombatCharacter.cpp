// Fill out your copyright notice in the Description page of Project Settings.

#include "C_CombatCharacter.h"
#include "ProjectARPG/Sturcts/FS_PostureStats.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/Animation/C_CombatAnim.h"
#include "../Camera/C_PlayerCameraManager.h"


AC_CombatCharacter::AC_CombatCharacter()
{

}

void AC_CombatCharacter::applyHitStop(float fSlowlate, float fDuration)
{
	UAnimInstance* pAnim = GetMesh()->GetAnimInstance();

	if (!pAnim)
		return;

	UAnimMontage* pMontage{};

	pMontage = pAnim->GetCurrentActiveMontage();

	if (!pMontage)
		return;

	GetWorldTimerManager().ClearTimer(m_hitStopTimerHandle);

	m_lastMontage = pMontage;

	m_fOriginalPlayRate = pAnim->Montage_GetPlayRate(pMontage);



	pAnim->Montage_SetPlayRate(pMontage, fSlowlate);
	UE_LOG(LogTemp, Warning, TEXT("HitStop start: rate=%.3f duration=%.3f"), fSlowlate, fDuration);

	GetWorldTimerManager().SetTimer
	(
		m_hitStopTimerHandle,
		this,
		&AC_CombatCharacter::endHitStop,
		fDuration,
		false
	);

}

void AC_CombatCharacter::endHitStop()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	UAnimMontage* Montage = Anim->GetCurrentActiveMontage();
	if (!Montage) return;

	if (m_lastMontage)
	{
		Anim->Montage_SetPlayRate(m_lastMontage, m_fOriginalPlayRate);
	}
	UE_LOG(LogTemp, Warning, TEXT("HitStop END: rate=%.3f"), m_fOriginalPlayRate);

	m_lastMontage = nullptr;
}

void AC_CombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_bIsPostureBroken)
		return;

	// 회복 지연 처리
	if (m_bIsRecoveryDelay)
	{
		m_fRecoveryDelayTimer -= DeltaTime;
		if (m_fRecoveryDelayTimer <= 0.f)
		{
			m_bIsRecoveryDelay = false;
		}
		else
		{
			return; // 지연 중에는 체간 회복 안 함
		}
	}

	// 체간 회복
	if (m_fCurrentPosture < m_fMaxPosture)
	{
		m_fCurrentPosture = FMath::Min(m_fMaxPosture, m_fCurrentPosture + m_fRecoveryRate * DeltaTime);
		m_OnPostureChanged.Broadcast(m_fCurrentPosture, m_fMaxPosture);
	}


}

void AC_CombatCharacter::setCombatState(E_CombatState eNewState)
{
	if (m_eState != eNewState)
		m_eState = eNewState;
}

E_CombatState AC_CombatCharacter::getCombatState() const
{
	return m_eState;
}

bool AC_CombatCharacter::isGuardingFront(AActor* pAttacker) const
{
	if (!pAttacker)
		return false;

	FVector vAttacker = (pAttacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector vFront = GetActorForwardVector();


	float fDot = FVector::DotProduct(vFront, vAttacker);

	return (fDot > -0.3f);
}

void AC_CombatCharacter::setGuard(bool bSet)
{
	m_bIsGuarding = bSet;
}

bool AC_CombatCharacter::isGuard() const
{
	return m_bIsGuarding;
}

void AC_CombatCharacter::setHp(float fHp)
{
	m_fCurrentHp = fHp;
}

float AC_CombatCharacter::getHp() const
{
	return m_fCurrentHp;
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
	m_HitActors.Empty();
	m_bIsTracing = false;
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

	const int32 nNumSubSteps = 3;
	for (int32 i = 0; i < nNumSubSteps; ++i)
	{
		float t = (float)i / (float)nNumSubSteps;
		FVector InterpStart = FMath::Lerp(vPrevStart, vCurStart, t);
		FVector InterpEnd = FMath::Lerp(vPrevEnd, vCurEnd, t);


		bool bHit = GetWorld()->SweepMultiByChannel(
			HitRes,
			InterpStart,
			InterpEnd,
			FQuat::Identity,
			ECC_GameTraceChannel3,
			FCollisionShape::MakeSphere(m_fTraceRadius),
			Params
		);



#if WITH_EDITOR

		DrawDebugCapsule(GetWorld(), (vPrevStart + vCurEnd) * 0.5f, FVector::Distance(vPrevStart, vCurEnd) * 0.5f,
			m_fTraceRadius, FQuat::Identity, bHit ? FColor::Green : FColor::Red, false, 0.05f);
	
#endif 
		bool bGuardSuccess = false;
		bool bAppliedHitStop = false;

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
					if (!bAppliedHitStop)
					{
						applyHitStop(0.01f, 0.09f);  // 공격자
						bAppliedHitStop = true;
						m_CamMgr->playHitCameraShake(0.3f);
					}

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

					

					if (AC_CombatCharacter* pTarget = Cast<AC_CombatCharacter>(pHitActor))
					{				

						if (pTarget->getCombatState() == E_CombatState::Guard)
						{
							bool bFront = pTarget->isGuardingFront(this);

							if (bFront)
							{
								fFinalDamage *= 0.1f;
								fFinalPostureDamage *= 0.5f;
								bGuardSuccess = true;
							}
							
						}

						if (bGuardSuccess)
						{
							// 공격자 방향과 반대 방향으로 살짝 밀기
							FVector KnockBackDir = (pTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
							KnockBackDir.Z = 0.f; // 위로 안 튀게
							float KnockBackStrength = 150.f; // 거리 조절

							pTarget->LaunchCharacter(KnockBackDir * KnockBackStrength, true, false);
						}
					}
					UE_LOG(LogTemp, Warning, TEXT("[%s] Hit %s!"), *GetName(), *pHitActor->GetName());
					IC_CombatInterface::Execute_takeDamage(pHitActor, fFinalDamage, fFinalPostureDamage, bGuardSuccess, this);
						
				}

				m_HitActors.Add(pHitActor);
			}
		}
	}

}

void AC_CombatCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker)
{
	if (m_fCurrentHp > 0)
		reduceHp(fDamage);

	m_OnHpChanged.Broadcast(m_fCurrentHp, m_fMaxHp);
	UE_LOG(LogTemp, Warning, TEXT("TakeDamage: HP %.1f / %.1f"), m_fCurrentHp, m_fMaxHp);



	// Posture 처리
	if (m_fCurrentPosture > 0.f)
	{
		reducePosture(fPostureDamage);

		// 체간 회복 지연 초기화
		m_bIsRecoveryDelay = true;
		m_fRecoveryDelayTimer = m_sPostureStats->fRecoveryDelay;

		m_OnPostureChanged.Broadcast(m_fCurrentPosture, m_fMaxPosture);
	}

	// 체간 붕괴 처리
	if (m_fCurrentPosture <= 0.f && !m_bIsPostureBroken)
	{
		m_fCurrentPosture = 0.f;
		m_OnPostureChanged.Broadcast(m_fCurrentPosture, m_fMaxPosture);

		onPostureBroken();
	}

	if (!bGuardSuccess)
	{
		E_Direction eDir = getHitDirection(pAttacker);

		playHitMontage(eDir);
		applyHitStop(0.01f, 0.12f);
		

	}
	else if (bGuardSuccess)
	{
		applyHitStop(0.05f, 0.02f); // 거의 체감 안 나는 히트스탑
		m_CamMgr->playHitCameraShake(0.2f);   // 흔들림 약하게
	}
		
}

void AC_CombatCharacter::onPostureBroken()
{
	if (m_bIsPostureBroken)
		return;

	m_bIsPostureBroken = true;

	m_bIsRecoveryDelay = true;

	if (m_pExecutionCom)
		m_pExecutionCom->playStunMontage();

	GetWorldTimerManager().SetTimer(m_timerHandle_PostureBroken, [this]() 
		{
			m_bIsPostureBroken = false;

		}, m_fBrokenDuration, false);
}

void AC_CombatCharacter::reduceHp(float fDamage)
{
	m_fCurrentHp = FMath::Clamp(m_fCurrentHp - fDamage, 0.f, m_fMaxHp);
}

void AC_CombatCharacter::reducePosture(float fDamage)
{
	m_fCurrentPosture = FMath::Clamp(m_fCurrentPosture - fDamage, 0.f, m_fMaxPosture);
}

void AC_CombatCharacter::playHitMontage(E_Direction eDir)
{
	if (UC_CombatAnim* pAnim = Cast<UC_CombatAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->playHitMontage(eDir);
	}
}

E_Direction AC_CombatCharacter::getHitDirection(AActor* pAttacker)
{
	FVector AttackDir = (pAttacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector Forward = GetActorForwardVector();

	float Dot = FVector::DotProduct(Forward, AttackDir);
	float CrossZ = FVector::CrossProduct(Forward, AttackDir).Z;

	if (Dot > 0.5f)      return E_Direction::Forward;
	if (Dot < -0.5f)     return E_Direction::Backward;
	if (CrossZ > 0.f)    return E_Direction::Right;


	return E_Direction::Left;

}


FVector AC_CombatCharacter::getLocation_Implementation()
{
	return GetActorLocation();
}

void AC_CombatCharacter::tryParry_Implementation(AActor* ParryOwner)
{
	if (!m_pParryCom) return;

	m_eState = E_CombatState::Parrying;

	if (m_pParryCom->isCanParry())
	{
		// 플레이어에게 알림
		IC_CombatInterface::Execute_onParrySuccess(ParryOwner, this);

		m_bWasParried = true;

		UAnimInstance* pAnim = GetMesh()->GetAnimInstance();
		if (!pAnim) return;

		// 현재 공격 중단
		if (pAnim->IsAnyMontagePlaying())
			pAnim->Montage_Stop(0.1f);

		// 패링 피격 모션
		if (m_pParryCom->m_ParriedTargetMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Playing ParriedTargetMontage"), *GetName());
			pAnim->Montage_Play(m_pParryCom->m_ParriedTargetMontage, 1.0f);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] ParriedTargetMontage is NULL!"), *GetName());
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] Got Parried by %s"), *GetName(), *ParryOwner->GetName());
	}
}

void AC_CombatCharacter::onParrySuccess_Implementation(AActor* ParryTarget)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Successfully parried %s!"), *GetName(), *ParryTarget->GetName());

	if (!m_pParryCom) return;

	AC_CombatCharacter* pTarget = Cast<AC_CombatCharacter>(ParryTarget);
	
	if (!pTarget)
		return;


	UAnimInstance* pAnim = GetMesh()->GetAnimInstance();
	if (!pAnim) return;

	// 현재 재생 중단
	if (pAnim->IsAnyMontagePlaying())
		pAnim->Montage_Stop(0.1f);

	// 패링 성공 모션
	if (m_pParryCom->m_ParryOwnerMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Playing ParryOwnerMontage"), *GetName());
		pAnim->Montage_Play(m_pParryCom->m_ParryOwnerMontage, 1.0f);
		applyHitStop(0.01f, 0.12f);


		pTarget->applyHitStop(0.01f, 0.12f);
		m_CamMgr->playHitCameraShake(1.5f);
		m_CamMgr->executionEffect(1.f);


		IC_CombatInterface::Execute_takeDamage(pTarget, 0.f, m_fAttackDamage, false, this);
		if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pTarget)) {

			FTimerHandle Timer;
			GetWorld()->GetTimerManager().SetTimer(
				Timer,
				FTimerDelegate::CreateLambda([this, pEnemy]()
					{
						if (pEnemy && pEnemy->canBeExecuted())
						{
							this->m_pExecutionCom->triggerExecution(pEnemy);
						}
					}),
				0.12f,
				false
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ParryOwnerMontage is NULL!"), *GetName());
	}

	

	m_eState = E_CombatState::Idle;

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
			m_fRecoveryDelayTimer = m_sPostureStats->fRecoveryDelay;
			m_fBrokenDuration = m_sPostureStats->fBrokenDuration;
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Posture Stats Row '%s' not found!"), *m_sPostureRowName.ToString());
		}
	}

	m_fCurrentHp = m_fMaxHp;

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

	m_pParryCom = GetComponentByClass<UC_ParryComponent>();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		m_CamMgr = Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager);
	}
}


