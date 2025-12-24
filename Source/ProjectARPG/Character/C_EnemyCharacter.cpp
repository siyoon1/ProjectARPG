// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyCharacter.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"
#include "ProjectARPG/Animation/C_EnemyAnim.h"
#include "ProjectARPG/AI/C_DetectComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"

void AC_EnemyCharacter::guardForDuration(float fTime)
{
	m_bIsExecutingAction = true;
	setGuard(true);

	GetWorldTimerManager().SetTimer(
		m_guardHandle,
		this,
		&AC_EnemyCharacter::endGuard,
		fTime,
		false
	);
}

void AC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_DetectCom = GetComponentByClass<UC_DetectComponent>();
}

E_EnemyCombatAction AC_EnemyCharacter::decideCombatAction() const
{
	APawn* pPlayer = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!pPlayer)
		return E_EnemyCombatAction::Guard;

	AC_CombatCharacter* pTarget = Cast<AC_CombatCharacter>(pPlayer);
	if (!pTarget)
		return E_EnemyCombatAction::Guard;

	bool bPlayerAttacking = pTarget->getCombatState() == E_CombatState::Attacking;
	bool bPlayerGuarding = pTarget->getCombatState() == E_CombatState::Guard;

	// 1? 플레이어 공격 중 → 대응
	if (bPlayerAttacking)
	{
		if (m_pParryCom && m_pParryCom->isCanParry() && FMath::FRand() < 0.35f)
			return E_EnemyCombatAction::Parry;

		return E_EnemyCombatAction::Guard;
	}

	// 2? 플레이어 가드 중 → 압박
	if (bPlayerGuarding)
	{
		return E_EnemyCombatAction::Attack;
	}

	// 3? 기본 상태 → 거의 항상 공격
	if (FMath::FRand() < 0.85f)
		return E_EnemyCombatAction::Attack;

	// 4? 아주 드물게 숨 고르기
	return E_EnemyCombatAction::Guard;
}

E_EnemyAttackType AC_EnemyCharacter::decideAttackType() const
{
	float fRan = FMath::FRand();

	if (fRan < 0.5f)
		return E_EnemyAttackType::Light;

	if (fRan < 0.8f)
		return E_EnemyAttackType::Heavy;

	return E_EnemyAttackType::Thrust;
}

void AC_EnemyCharacter::showHpBar(bool bShow)
{
	m_wHpBarCom = GetComponentByClass<UWidgetComponent>();

	if (m_wHpBarCom)
		m_wHpBarCom->SetVisibility(bShow);
}

void AC_EnemyCharacter::showExecutionVFX(bool bShow)
{
	if (!m_ExecutionVFX)
		m_ExecutionVFX = GetComponentByClass<UNiagaraComponent>();

	if (m_ExecutionVFX)
		m_ExecutionVFX->SetVisibility(bShow);
}

void AC_EnemyCharacter::setCanBeExecuted(bool bCan)
{
	m_bCanbeExcuted = bCan;
}

bool AC_EnemyCharacter::canBeExecuted() const
{
	return m_bCanbeExcuted && m_bIsPostureBroken;
}

bool AC_EnemyCharacter::isAnawareOfPlayer() const
{
	return m_DetectCom && m_DetectCom->isDetecting();
}

void AC_EnemyCharacter::onPostureBroken()
{
	Super::onPostureBroken();

	if (!m_bCanbeExcuted)
	{
		m_bCanbeExcuted = true;

		if (m_pExecutionCom)
		{
			m_pExecutionCom->onBecomeExecutable(this);
			UE_LOG(LogTemp, Error, TEXT("ONBECOMEEXCUTABLE!!!!"));
		}


	}

}

void AC_EnemyCharacter::executeCombatAction()
{
	if (m_bIsPostureBroken)
		return;

	E_EnemyCombatAction eAction = decideCombatAction();
	m_eCurrentAction = eAction;

	switch (eAction)
	{
	case E_EnemyCombatAction::Attack:
		attack();
		break;
	case E_EnemyCombatAction::Guard:
		guardForDuration(0.6f);
		break;
	case E_EnemyCombatAction::Parry:tryParry(this);
		break;

	case E_EnemyCombatAction::Wait:
	default:
		break;
	}
}

bool AC_EnemyCharacter::isExecutingAction() const
{
	return m_bIsExecutingAction;
}

bool AC_EnemyCharacter::isActionFinished() const
{
	return !m_bIsExecutingAction;
}

void AC_EnemyCharacter::endGuard()
{
	setGuard(false);
	m_bIsExecutingAction = false;
}

void AC_EnemyCharacter::endAttack()
{
	m_bIsExecutingAction = false;
}

void AC_EnemyCharacter::attack()
{
	m_bIsExecutingAction = true;

	m_eCurrentAttackType = decideAttackType();

	/*if (auto* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
	}*/

	APawn* target = Cast<APawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (target)
	{
		FVector toTarget = (target->GetActorLocation() - GetActorLocation());
		toTarget.Z = 0.f; // 상하 무시
		FRotator lookRot = toTarget.Rotation();
		SetActorRotation(lookRot);
	}

	if (UAnimInstance* pAnim = GetMesh()->GetAnimInstance())
	{
		if (UC_EnemyAnim* pEnemyAnim = Cast<UC_EnemyAnim>(pAnim))
		{
			pEnemyAnim->playAttackByType(m_eCurrentAttackType);
		}
	}
}

void AC_EnemyCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker)
{
	Super::takeDamage_Implementation(fDamage, fPostureDamage, bGuardSuccess, pAttacker);

	if (UC_DetectComponent* pDetect = FindComponentByClass<UC_DetectComponent>())
	{
		pDetect->forceDetect(pAttacker);
	}
}
