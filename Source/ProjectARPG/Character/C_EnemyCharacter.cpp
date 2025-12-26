// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyCharacter.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"
#include "ProjectARPG/Animation/C_EnemyAnim.h"
#include "ProjectARPG/AI/C_DetectComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "BrainComponent.h"


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

void AC_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!m_bInCombat)
		return;

	updateCombatAI(DeltaTime);

	if (!m_DetectCom || !m_DetectCom->isDetecting())
	{
		setInCombat(false);
		return;
	}
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

void AC_EnemyCharacter::updateCombatAI(float fDelta)
{
	if (m_bIsExecutingAction)
		return;

	if (GetWorld()->GetTimeSeconds() < m_nextActionTime)
		return;

	executeCombatAction();
}

void AC_EnemyCharacter::executeCombatAction()
{
	if (m_bIsDead)
		return;

	if (m_bIsPostureBroken)
		return;

	if (m_bIsExecutingAction)
		return;

	m_bActionStarted = false;

	E_EnemyCombatAction eAction = decideCombatAction();
	m_eCurrentAction = eAction;

	switch (eAction)
	{
	case E_EnemyCombatAction::Attack:
		attack();
		m_bActionStarted = true;
		break;
	case E_EnemyCombatAction::Guard:
		guardForDuration(0.6f);
		m_bActionStarted = true;
		break;
	case E_EnemyCombatAction::Parry:
		tryParry(this);
		m_bActionStarted = true;
		break;

	default:
		break;
	}
}

void AC_EnemyCharacter::setInCombat(bool bEnable)
{
	m_bInCombat = bEnable;
}

bool AC_EnemyCharacter::isExecutingAction() const
{
	return m_bIsExecutingAction;
}

void AC_EnemyCharacter::endGuard()
{
	setGuard(false);
	m_bIsExecutingAction = false;
	m_nextActionTime = GetWorld()->GetTimeSeconds() + m_actionInterval;
}

void AC_EnemyCharacter::endAttack()
{
	m_bIsExecutingAction = false;
	m_nextActionTime = GetWorld()->GetTimeSeconds() + m_actionInterval;
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

bool AC_EnemyCharacter::isGuard() const
{
	return m_bIsGuarding;
}

void AC_EnemyCharacter::onParryFinished()
{
	m_bIsExecutingAction = false;
	m_eState = E_CombatState::Idle;

	m_nextActionTime = GetWorld()->GetTimeSeconds() + 0.3f;
}

void AC_EnemyCharacter::onDeath()
{
	Super::onDeath();

	if (AAIController* pAiCon = Cast<AAIController>(GetController()))
	{
		pAiCon->StopMovement();
		pAiCon->BrainComponent->StopLogic(TEXT("Dead"));
	}

	showExecutionVFX(false);


}

void AC_EnemyCharacter::takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker)
{
	Super::takeDamage_Implementation(fDamage, fPostureDamage, bGuardSuccess, pAttacker);

	if (UC_DetectComponent* pDetect = FindComponentByClass<UC_DetectComponent>())
	{
		pDetect->forceDetect(pAttacker);
	}
}

void AC_EnemyCharacter::tryParry_Implementation(AActor* ParryOwner)
{
	Super::tryParry_Implementation(ParryOwner);

	m_bIsExecutingAction = true;
}
