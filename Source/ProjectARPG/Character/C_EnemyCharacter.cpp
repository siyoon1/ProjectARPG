// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyCharacter.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "ProjectARPG/ActorComponents/C_ExecutionComponent.h"
#include "ProjectARPG/Animation/C_EnemyAnim.h"
#include "ProjectARPG/AI/C_DetectComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "ProjectARPG/ActorComponents/C_EnemyAttackComponent.h"


void AC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_DetectCom = GetComponentByClass<UC_DetectComponent>();

	m_pPlayer = Cast< AC_CombatCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	applyCombatProfile();

	showHpBar(false);

	m_EnemyAttackComp = FindComponentByClass<UC_EnemyAttackComponent>();
}

bool AC_EnemyCharacter::guardForDuration(float fTime)
{
	if (!canDecideAction())
		return false;

	beginAction();

	setGuard(true);

	GetWorldTimerManager().SetTimer(
		m_guardHandle,
		this,
		&AC_EnemyCharacter::endGuard,
		fTime,
		false
	);

	return true;
}

void AC_EnemyCharacter::applyCombatProfile()
{
	const FS_EnemyCombatProfile* profile = m_CombatProfiles.Find(m_eEnemyTier);

	if (!profile)
		return;

	m_CurrentCombatProfile = *profile;

}

float AC_EnemyCharacter::getDistToTarget() const
{
	if (!m_pPlayer)
		return TNumericLimits<float>::Max();

	return FVector::Dist(
		GetActorLocation(),
		m_pPlayer->GetActorLocation()
	);
}


bool AC_EnemyCharacter::startGuard()
{
	if (m_bIsGuarding)
		return false;

	beginAction();
	setGuard(true);

	m_bIsGuarding = true;

	m_fGuardStartTime = GetWorld()->GetTimeSeconds();

	return true;
}

bool AC_EnemyCharacter::canReleaseGuard() const
{
	const float fNow = GetWorld()->GetTimeSeconds();
	const float fElapsed = fNow - m_fGuardStartTime;


	if (fElapsed < m_CurrentCombatProfile.fGuardMinTime)
		return false;

	if (fElapsed >= m_CurrentCombatProfile.fGuardMaxTime)
		return true;

	return true;
}

void AC_EnemyCharacter::endGuard()
{
	setGuard(false);
	m_bIsGuarding = false;

	const float Cooldown = m_CurrentCombatProfile.fActionInterval;
	finishAction(Cooldown);
}

bool AC_EnemyCharacter::canDecideAction() const
{
	return m_EnemyActionState == E_EnemyActionState::Idle;
}

void AC_EnemyCharacter::beginAction()
{
	m_EnemyActionState = E_EnemyActionState::Executing;
}


void AC_EnemyCharacter::finishAction(float fCooldown)
{
	m_EnemyActionState = E_EnemyActionState::Cooldown;
	m_nextActionTime = GetWorld()->GetTimeSeconds() + fCooldown;

	GetWorldTimerManager().SetTimer(
		m_actionCooldownHandle,
		this,
		&AC_EnemyCharacter::onActionCooldownFinished,
		fCooldown,
		false
	);
}

void AC_EnemyCharacter::onActionCooldownFinished()
{
	m_EnemyActionState = E_EnemyActionState::Idle;
}

bool AC_EnemyCharacter::tryAttack()
{
	if (!canDecideAction())
		return false;

	if (!m_EnemyAttackComp)
		return false;

	const float Dist = getDistToTarget();

	return m_EnemyAttackComp->tryExecuteAttack(Dist);

}

bool AC_EnemyCharacter::playAttack(const FS_AttackData* Data)
{
	if (!Data)
		return false;

	if (m_pPlayer)
	{
		FVector dir = m_pPlayer->GetActorLocation() - GetActorLocation();
		dir.Z = 0;
		SetActorRotation(dir.Rotation());
	}

	/*if (UAnimInstance* pAnim = GetMesh()->GetAnimInstance())
	{
		if (UC_EnemyAnim* pEnemyAnim = Cast<UC_EnemyAnim>(pAnim))
		{
			pEnemyAnim->playAttackMontage(pAttackData->pMontage);
		}
	}*/

	return true;
}


void AC_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UC_EnemyAttackComponent* AC_EnemyCharacter::getAttackComponent() const
{
	return m_EnemyAttackComp;
}

const FS_AttackRuntimeState* AC_EnemyCharacter::getAttackRuntimeState(FName Row) const
{
	return m_AttackStates.Find(Row);
}

void AC_EnemyCharacter::markAttackUsed(FName Row, float Cooldown)
{
	FS_AttackRuntimeState& State = m_AttackStates.FindOrAdd(Row);
	State.LastUsedTime = GetWorld()->GetTimeSeconds();
	State.Cooldown = Cooldown;
}

bool AC_EnemyCharacter::playStepBack()
{
	if (!canDecideAction())
		return false;

	beginAction();

	if (UC_EnemyAnim* pAnim = Cast< UC_EnemyAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->playStepBackMontage();
	}

	return true;
}

void AC_EnemyCharacter::endStepBack()
{
	const float Cooldown = m_CurrentCombatProfile.fActionInterval;
	finishAction(Cooldown);

	m_onStepBackFinished.Broadcast();
}


void AC_EnemyCharacter::endAttack()
{
	const float Cooldown = m_CurrentCombatProfile.fActionInterval;
	finishAction(Cooldown);

	m_onAttackFinished.Broadcast();
}

void AC_EnemyCharacter::showHpBar(bool bShow)
{
	m_wHpBarCom = GetComponentByClass<UWidgetComponent>();

	if (m_eEnemyTier == E_EnemyTier::Boss || m_eEnemyTier == E_EnemyTier::MiniBoss)
		return;

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

void AC_EnemyCharacter::onCombatStarted()
{
	setInCombat(true);
	showHpBar(true);
}

void AC_EnemyCharacter::onCombatEnded()
{
	setInCombat(false);
	showHpBar(false);
}

float AC_EnemyCharacter::getNextActionTime() const
{
	return m_nextActionTime;
}

FS_EnemyCombatProfile& AC_EnemyCharacter::getCombatProfile()
{
	return m_CurrentCombatProfile;
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

		/*if (m_pExecutionCom)
		{
			m_pExecutionCom->onBecomeExecutable(this);
			UE_LOG(LogTemp, Error, TEXT("ONBECOMEEXCUTABLE!!!!"));
		}*/


	}

}

void AC_EnemyCharacter::setInCombat(bool bCombat)
{
	if (m_bInCombat == bCombat)
		return;

	m_bInCombat = bCombat;

	if (!isBoss())
	{
		showHpBar(bCombat);
	}
	else
	{
		m_onBossStateChanged.Broadcast(this, bCombat);
	}
}

bool AC_EnemyCharacter::isExecutingAction() const
{
	return m_bIsExecutingAction;
}

bool AC_EnemyCharacter::isGuard() const
{
	return m_bIsGuarding;
}

bool AC_EnemyCharacter::isCombat() const
{
	return m_bInCombat;
}

void AC_EnemyCharacter::onParryFinished()
{
	finishAction(0.f);
	m_bIsExecutingAction = false;
	m_eState = E_CombatState::Idle;

	m_nextActionTime = GetWorld()->GetTimeSeconds() + 0.3f;
}

bool AC_EnemyCharacter::isBoss() const
{
	return m_eEnemyTier == E_EnemyTier::MiniBoss || m_eEnemyTier == E_EnemyTier::Boss;
}

void AC_EnemyCharacter::onExecuted()
{
	Super::onExecuted();

	m_bIsExecutingAction = false;
	m_bInCombat = true;
	m_bCanbeExcuted = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	m_nextActionTime = GetWorld()->GetTimeSeconds() + 0.1f;
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

void AC_EnemyCharacter::onExecutionStarted()
{
	setCanBeExecuted(false);
	GetCharacterMovement()->DisableMovement();

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		AICon->BrainComponent->StopLogic(TEXT("Executed"));
	}


}
