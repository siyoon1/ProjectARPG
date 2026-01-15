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
#include "Components/CapsuleComponent.h"


AC_CombatCharacter::AC_CombatCharacter()
{

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

	m_CurrentLifeNodes = m_MaxLifeNodes;

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

void AC_CombatCharacter::applyHitStop(float fSlowlate, float fDuration)
{
	if (m_bHitStopActive)
		return;

	if (!canAct())
		return;

	UAnimInstance* pAnim = GetMesh()->GetAnimInstance();

	if (!pAnim)
		return;

	UAnimMontage* pMontage{};

	pMontage = pAnim->GetCurrentActiveMontage();

	if (!pMontage)
		return;

	m_bHitStopActive = true;

	
	m_lastMontage = pMontage;

	m_fOriginalPlayRate = pAnim->Montage_GetPlayRate(pMontage);


	pAnim->Montage_SetPlayRate(pMontage, fSlowlate);
	UE_LOG(LogTemp, Warning, TEXT("HitStop start: rate=%.3f duration=%.3f"), fSlowlate, fDuration);

	GetWorldTimerManager().ClearTimer(m_hitStopTimerHandle);
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
	m_bHitStopActive = false;
}

void AC_CombatCharacter::setRuntimeParryDir(E_ParryDirection eDir)
{
	m_RuntimeParryDir = eDir;
}

E_ParryDirection AC_CombatCharacter::getCurrentParryDir() const
{
	if (m_RuntimeParryDir != E_ParryDirection::None)
		return m_RuntimeParryDir;

	const FS_AttackData* Attack = getCurrentAttackData();
	return Attack ? Attack->eParryDirection : E_ParryDirection::Both;
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

const FS_AttackData* AC_CombatCharacter::getAttackData(FName RowName) const
{
	if (RowName.IsNone())
		return nullptr;

	if (!m_pAttackDataTable)
		return nullptr;


	return m_pAttackDataTable->FindRow<FS_AttackData>(RowName, TEXT("getAttackData"));
}

const FS_AttackData* AC_CombatCharacter::getCurrentAttackData() const
{
	return m_pCurrentAttackData;
}

void AC_CombatCharacter::applyAttack(const FS_AttackData& sData)
{
	m_pCurrentAttackData = &sData;

	m_fAttackDamage = sData.fDamage;
	m_fPostureDamage = sData.fPostureDamage;

	m_bCurrentAttackUnblockable = sData.bUnblockable;
	m_bCurrentAttackCanParry = sData.bCanParry;
	m_fGuardPushBack = sData.fGuardPushBack;

	switch (sData.eProperty)
	{
	case E_AttackProperty::Normal:
		m_eAttackType = E_AttackType::Normal;
		break;

	case E_AttackProperty::Heavy:
		m_eAttackType = E_AttackType::Normal;
		break;

	case E_AttackProperty::Thrust:
	case E_AttackProperty::Sweep:
		m_eAttackType = E_AttackType::Charge;
		break;
	}
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

bool AC_CombatCharacter::canAct() const
{
	return m_ActionState == E_ActionState::Free;
}

bool AC_CombatCharacter::isInvincibleAgainst(AActor* pAttacker) const
{
	if (isDead())
		return true;

	if (m_eState == E_CombatState::Executing)
		return true;

	// 인살 가능 상태 (HP 0 대기 상태)
	if (m_bExecutionAvailable)
		return true;

	return false;
}

void AC_CombatCharacter::onExecuted()
{
	if (isDead())
		return;

	m_CurrentLifeNodes--;

	m_OnLifeNodeChanged.Broadcast(m_CurrentLifeNodes, m_MaxLifeNodes);

	if (m_CurrentLifeNodes <= 0)
	{
		onDeath();
		return;
	}

	m_bExecutionAvailable = false;
	m_bIsPostureBroken = false;
	m_bIsRecoveryDelay = false;

	GetWorldTimerManager().ClearTimer(m_timerHandle_PostureBroken);

	m_fCurrentPosture = m_fMaxPosture;
	m_OnPostureChanged.Broadcast(m_fCurrentPosture, m_fMaxPosture);

	m_fCurrentHp = m_fMaxHp;
	m_OnHpChanged.Broadcast(m_fCurrentHp, m_fMaxHp);

	m_eState = E_CombatState::Idle;

	
	
}

void AC_CombatCharacter::setHp(float fHp)
{
	m_fCurrentHp = fHp;
}

float AC_CombatCharacter::getHp() const
{
	return m_fCurrentHp;
}

void AC_CombatCharacter::setMaxHp(float fHp)
{
	m_fMaxHp = fHp;
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
	if (!canAct())
		return;

	if (m_bIsTracing)
		return;

	m_HitActors.Empty();
	m_bIsTracing = true;
	

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
	if (isDead())
		return;

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
					bool bValidHit = true;


					AC_CombatCharacter* pTarget = Cast<AC_CombatCharacter>(pHitActor);

					if (pTarget)
					{
						// 죽은 적 무시
						if (pTarget->isDead())
							bValidHit = false;

						// 인살 중 / 무적 상태면 무시 (확장 포인트)
						if (pTarget->isInvincibleAgainst(this))
							bValidHit = false;
					}

					if (!bValidHit)
					{
						m_HitActors.Add(pHitActor);
						continue;
					}


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

					
					bool bGuardSuccess = false;

					if (pTarget)
					{			
						if (pTarget->m_bIsGuarding == true || pTarget->getCombatState() == E_CombatState::Guard)
						{
							bool bFront = pTarget->isGuardingFront(this);

							if (bFront)
							{
								bGuardSuccess = true;

								fFinalDamage *= 0.1f;
								fFinalPostureDamage *= 0.5f;

								if (m_fGuardPushBack > 0.f)
								{
									FVector vDir = (pTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
									vDir.Z = 0.f;
									pTarget->LaunchCharacter(vDir * m_fGuardPushBack, true, false);
								}
								
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
	if (isInvincibleAgainst(pAttacker))
		return;

	reduceHp(fDamage);

	if (m_fCurrentHp <= 0.f)
	{
		m_fCurrentHp = 0.f;

		m_OnHpChanged.Broadcast(m_fCurrentHp, m_fMaxHp);

		enterExecutionReady();
		return;
	}

	

	m_OnHpChanged.Broadcast(m_fCurrentHp, m_fMaxHp);

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

		if (!m_bIsPostureBroken)
		{
			E_Direction eDir = getHitDirection(pAttacker);

			playHitMontage(eDir);
		}
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
	m_ActionState = E_ActionState::Stunned;
	m_CombatMode = E_CombatMode::None;

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

void AC_CombatCharacter::enterExecutionReady()
{
	if (m_bExecutionAvailable || isDead())
		return;

	m_bExecutionAvailable = true;
	stopAttackTrace();

	onPostureBroken();
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

	if (!m_pParryCom->canParry())
		return;

	m_eState = E_CombatState::Parrying;
	
	IC_CombatInterface::Execute_onParrySuccess(ParryOwner, this);


	UAnimInstance* pAnim = GetMesh()->GetAnimInstance();
	if (!pAnim) 
		return;

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

	m_pParryCom->closeParry();

	UE_LOG(LogTemp, Warning, TEXT("[%s] Got Parried by %s"), *GetName(), *ParryOwner->GetName());
	
}

void AC_CombatCharacter::onParrySuccess_Implementation(AActor* ParryTarget)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Successfully parried %s!"), *GetName(), *ParryTarget->GetName());

	const FS_AttackData* Attack{};
	
	AC_CombatCharacter* pTarget = Cast<AC_CombatCharacter>(ParryTarget);
	
	if (!pTarget)
		return;

	Attack = pTarget->getCurrentAttackData();

	if (!Attack)
		return;

	// 패링 성공자
	if (UAnimInstance* PlayerAnim = GetMesh()->GetAnimInstance())
	{
		if (UC_CombatAnim* pAnim = Cast<UC_CombatAnim>(PlayerAnim))
		{
			pAnim->playParryMontage(Attack->eParryDirection);

			applyHitStop(0.01f, 0.12f);
			m_CamMgr->playHitCameraShake(1.5f);
			m_CamMgr->executionEffect(1.f);
		}

		
	}

	// 패링 당한 쪽
	if (pTarget->m_pParryCom)
	{
		if (UAnimInstance* EnemyAnim =
			pTarget->GetMesh()->GetAnimInstance())
		{
			if (pTarget->m_pParryCom->m_ParriedTargetMontage)
			{
				EnemyAnim->Montage_Play(
					pTarget->m_pParryCom->m_ParriedTargetMontage,
					1.0f
				);
			}
		}
	}


	IC_CombatInterface::Execute_takeDamage(
		pTarget,
		0.f,
		m_fAttackDamage,
		false,
		this
	);

	if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pTarget))
	{
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(
			Timer,
			FTimerDelegate::CreateLambda([this, pEnemy]()
				{
					if (pEnemy && pEnemy->canBeExecuted())
					{
						m_pExecutionCom->triggerExecution(
							pEnemy,
							E_ExecutionType::PostureBreak
						);
					}
				}),
			0.12f,
			false
		);
	}
	

	m_eState = E_CombatState::Idle;

}

UC_ParryComponent* AC_CombatCharacter::getParryComponent() const
{
	return m_pParryCom;
}



void AC_CombatCharacter::onDeath()
{
	m_ActionState = E_ActionState::Dead;
	m_CombatMode = E_CombatMode::None;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	 
}

bool AC_CombatCharacter::isDead() const
{
	return m_ActionState == E_ActionState::Dead;
}


