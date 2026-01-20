// Fill out your copyright notice in the Description page of Project Settings.

#include "C_CombatCharacter.h"
#include "ProjectARPG/Sturcts/FS_PostureStats.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "ProjectARPG/Animation/C_CombatAnim.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/ActorComponents/C_CombatStatComponent.h"
#include "ProjectARPG/ActorComponents/C_AttackComponent.h"


AC_CombatCharacter::AC_CombatCharacter()
{
	m_StatComp = CreateDefaultSubobject<UC_CombatStatComponent>(TEXT("StatComp"));
	m_AttackComp = CreateDefaultSubobject<UC_AttackComponent>(TEXT("AttackComp"));
}

void AC_CombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_CurrentLifeNodes = m_MaxLifeNodes;

	if (!m_pTraceStart)
	{
		m_pTraceStart = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("TraceStart")));
	}

	if (!m_pTraceEnd)
	{
		m_pTraceEnd = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("TraceEnd")));
	}

	m_pParryCom = GetComponentByClass<UC_ParryComponent>();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		m_CamMgr = Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager);
	}

	m_StatComp->m_OnPostureBroken.AddUObject(this, &AC_CombatCharacter::onPostureBroken);
}


void AC_CombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


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

void AC_CombatCharacter::applyHitFeedback(E_HitResult HitResult, AActor* Attacker)
{
	if (AC_CombatCharacter* AttackerChar =
		Cast<AC_CombatCharacter>(Attacker))
	{
		AttackerChar->applyAttackerHitFeedback(HitResult, Attacker);
	}

	m_CamMgr->playHitCameraShake(0.2f);
}

void AC_CombatCharacter::applyAttackerHitFeedback(E_HitResult HitResult, AActor* Attacker)
{

	switch (HitResult)
	{
	case E_HitResult::Normal:
		applyHitStop(0.05f, 0.02f);
		m_CamMgr->playHitCameraShake(0.2f);
		break;

	case E_HitResult::Guarded:
		applyHitStop(0.03f, 0.015f);
		break;

	case E_HitResult::PostureBroken:
		applyHitStop(0.1f, 0.08f);
		break;
	}
	

}

void AC_CombatCharacter::startAttack(const FS_AttackData& AttackData)
{
	m_pCurrentAttackData = &AttackData;

	m_fAttackDamage = AttackData.Combat.Damage;
	m_fPostureDamage = AttackData.Combat.PostureDamage;
	m_fGuardPushBack = AttackData.Combat.GuardPushBack;

	if (m_AttackComp)
		m_AttackComp->startAttack(AttackData);
}

void AC_CombatCharacter::endAttack()
{
	if (m_AttackComp)
		m_AttackComp->endAttack();

	m_pCurrentAttackData = nullptr;
}

void AC_CombatCharacter::setCurrentAttackRow(FName RowName)
{
	m_CurrentAttackRow = RowName;
}

FName AC_CombatCharacter::getCurrentAttackRow() const
{
	return m_CurrentAttackRow;
}

void AC_CombatCharacter::setActionState(E_ActionState eNewState)
{
	if (m_ActionState != eNewState)
		m_ActionState = eNewState;
}

E_ActionState AC_CombatCharacter::getActionState() const
{
	return m_ActionState;
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

FVector AC_CombatCharacter::getTraceStartLocation() const
{
	return m_pTraceStart ? m_pTraceStart->GetComponentLocation()
		: GetActorLocation();
}

FVector AC_CombatCharacter::getTraceEndLocation() const
{
	return m_pTraceEnd ? m_pTraceEnd->GetComponentLocation()
		: GetActorLocation();
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

	//인살 연출 중
	if (m_CombatMode == E_CombatMode::Executing)
		return true;

	return false;
}

	/*if (isDead())
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

	GetWorldTimerManager().ClearTimer(m_timerHandle_PostureBroken);


	m_eState = E_CombatState::Idle;*/


float AC_CombatCharacter::getHp() const
{
	return m_StatComp->getCurrentHp();
}


float AC_CombatCharacter::getPosture() const
{
	return m_StatComp->getCurrentPosture();
}


void AC_CombatCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker)
{
	if (isInvincibleAgainst(pAttacker))
		return;

	m_StatComp->applyDamage(fDamage, fPostureDamage);

	E_HitResult HitResult = E_HitResult::Normal;

	if (bGuardSuccess)
		HitResult = E_HitResult::Guarded;
	else if (m_StatComp->isPostureBroken())
		HitResult = E_HitResult::PostureBroken;

	applyHitFeedback(HitResult, pAttacker);
	
}

void AC_CombatCharacter::onPostureBroken()
{
	UE_LOG(LogTemp, Warning,
		TEXT("[POSTURE] CombatCharacter::onPostureBroken %s"),
		*GetName());

	m_ActionState = E_ActionState::Stunned;
	m_CombatMode = E_CombatMode::None;

	onPostureBroken_Internal();

}

void AC_CombatCharacter::onPostureBroken_Internal()
{

}

void AC_CombatCharacter::enterExecutionReady()
{
	if (m_bExecutionAvailable || isDead())
		return;

	m_bExecutionAvailable = true;

	onPostureBroken();
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
	/*if (UAnimInstance* PlayerAnim = GetMesh()->GetAnimInstance())
	{
		if (UC_CombatAnim* pAnim = Cast<UC_CombatAnim>(PlayerAnim))
		{
			pAnim->playParryMontage(Attack->eParryDirection);

			applyHitStop(0.01f, 0.12f);
			m_CamMgr->playHitCameraShake(1.5f);
			m_CamMgr->executionEffect(1.f);
		}

		
	}*/

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

	//if (AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pTarget))
	//{
	//	FTimerHandle Timer;
	//	GetWorld()->GetTimerManager().SetTimer(
	//		Timer,
	//		FTimerDelegate::CreateLambda([this, pEnemy]()
	//			{
	//				if (pEnemy && pEnemy->canBeExecuted())
	//				{
	//					/*m_pExecutionCom->triggerExecution(
	//						pEnemy,
	//						E_ExecutionType::PostureBreak
	//					);*/
	//				}
	//			}),
	//		0.12f,
	//		false
	//	);
	//}
	

	m_eState = E_CombatState::Idle;

}

void AC_CombatCharacter::applyAttack(const FS_AttackData& AttackData)
{
	// TODO: 실제 공격 처리
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


