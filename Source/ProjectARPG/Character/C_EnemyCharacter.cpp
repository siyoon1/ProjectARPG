// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyCharacter.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "ProjectARPG/Animation/C_EnemyAnim.h"
#include "ProjectARPG/AI/C_DetectComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/ActorComponents/C_ParryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "ProjectARPG/ActorComponents/C_CombatStatComponent.h"
#include "ProjectARPG/Data/C_ExecutionReactionData.h"
#include "ProjectARPG/ActorComponents/C_AttackComponent.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"
#include "ProjectARPG/Interface/C_ExecutionRequester.h"


AC_EnemyCharacter::AC_EnemyCharacter()
{
	m_AIAttackComp = CreateDefaultSubobject<UC_AIAttackComponent>(TEXT("EnemyAttackComp"));
}

void AC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_DetectCom = GetComponentByClass<UC_DetectComponent>();

	m_pPlayer = Cast< AC_CombatCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	applyCombatProfile();

	setExecutionHintVisible(false);
	showHpBar(false);

	if (m_AIAttackComp == nullptr)
		UE_LOG(LogTemp, Error,TEXT("m_EnemyAttackComp NULL!!!"))
	else
		UE_LOG(LogTemp, Error, TEXT("m_EnemyAttackComp NOT NULL!!!"))

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

void AC_EnemyCharacter::applyExecutionFacing(const FS_ExecutionContext& Context)
{
	AActor* ExecutionInstigator = Context.Instigator.Get();

	if (!ExecutionInstigator)
		return;

	if (Context.Type == E_ExecutionType::Stealth)
		return;

	

	const FRotator LookAt = (ExecutionInstigator->GetActorLocation() - GetActorLocation()).Rotation();

	SetActorRotation(LookAt);
}




bool AC_EnemyCharacter::startGuard()
{
	if (!canDecideAction())
		return false;

	beginAction();

	if (!Super::startGuard())
		return false;

	m_fGuardStartTime = GetWorld()->GetTimeSeconds();
	return true;
}

void AC_EnemyCharacter::endGuard()
{
	Super::endGuard();

	const float Cooldown = m_CurrentCombatProfile.fActionInterval;
	finishAction(Cooldown);
}

bool AC_EnemyCharacter::canReleaseGuard() const
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - m_fGuardStartTime;

	if (Elapsed < m_CurrentCombatProfile.fGuardMinTime)
		return false;

	if (Elapsed >= m_CurrentCombatProfile.fGuardMaxTime)
		return true;

	// 중간 구간: AI 판단 여지
	return true;
}



bool AC_EnemyCharacter::canDecideAction() const
{
	return m_EnemyActionState == E_EnemyActionState::Idle && canAct();
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

bool AC_EnemyCharacter::playAttackByRow(FName AttackRow)
{
	const FS_AttackData* AttackData = getAttackData(AttackRow);

	if (!AttackData)
		return false;

	if (!canDecideAction())
		return false;

	beginAction();

	if (!m_AttackComp)
		return false;

	m_AttackComp->startAttack(*AttackData);

	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Enemy] Play Attack Montage Row=%s"),
			*AttackRow.ToString());

		Anim->Montage_Play(AttackData->Anim.Montage);
	}
	else
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Enemy] AnimInstance is NULL"));
		return false;
	}

	return true;
}

bool AC_EnemyCharacter::tryAttack()
{
	if (!canDecideAction())
		return false;

	if (!m_AIAttackComp)
		return false;

	const float Dist = getDistToTarget();

	return m_AIAttackComp->tryExecuteAttack(Dist);

}

bool AC_EnemyCharacter::playAttack(const FS_AttackData* AttackData)
{
	if (!AttackData || !m_AttackComp)
		return false;

	m_AttackComp->startAttack(*AttackData);

	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("Play Enemy Attack Montage"));
		Anim->Montage_Play(AttackData->Anim.Montage);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance is NULL"));
	}

	return true;
}


void AC_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	if (m_bInCombat)
		return;

	setInCombat(true);
	showHpBar(true);
}

void AC_EnemyCharacter::onCombatEnded()
{
	m_bInCombat = false;

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

bool AC_EnemyCharacter::canBeExecuted(E_ExecutionType Type) const
{
	switch (Type)
	{
	case E_ExecutionType::PostureBreak:
		return m_bCanBeExecuted && m_bIsPostureBroken;

	case E_ExecutionType::Stealth:
		return !isAnawareOfPlayer();

	default:
		return false;
	}
}

void AC_EnemyCharacter::onExecutionStarted(APawn* ExecutionInstigator, const FS_ExecutionContext& Context)
{
	m_bCanBeExecuted = false;

	// 이동 정지
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	// AI 정지
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();

		if (AICon->BrainComponent)
		{
			AICon->BrainComponent->StopLogic(TEXT("Execution"));
		}
	}

	// 전투 상태 차단
	setGuard(false);
	setInCombat(false);
	setExecutionHintVisible(false);

	UAnimMontage* Reaction =
		m_ReactionData->selectMontage(
			Context.Type,
			Context.Index);

	

	if (Reaction)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			applyExecutionFacing(Context);
			Anim->Montage_Play(Reaction);
		}
	}
}

void AC_EnemyCharacter::onExecutionFinished(APawn* ExecutionInstigator)
{
	onDeath();
}

void AC_EnemyCharacter::setExecutionHintVisible(bool bVisible)
{
	showExecutionVFX(bVisible);
}

void AC_EnemyCharacter::setCanBeExecuted(bool bCan)
{
	m_bCanBeExecuted = bCan;
}

bool AC_EnemyCharacter::canBeExecuted() const
{
	return m_bCanBeExecuted && m_bIsPostureBroken;
}

bool AC_EnemyCharacter::isAnawareOfPlayer() const
{
	return m_DetectCom && m_DetectCom->isDetecting();
}

void AC_EnemyCharacter::onPostureBroken()
{
	UE_LOG(LogTemp, Warning,
		TEXT("[POSTURE] Enemy::onPostureBroken %s"),
		*GetName());

	Super::onPostureBroken();

}

void AC_EnemyCharacter::onPostureBroken_Internal()
{
	m_bCanBeExecuted = true;
	m_bIsPostureBroken = true;
	setExecutionHintVisible(true);

	
	if (!m_StatComp)
		return;

	if (m_StatComp->getBreakCause() != E_PostureBreakCause::Parry)
		return;

	AActor* Breaker = m_StatComp->getLastBreak();
	if (!Breaker)
		return;

	if (Breaker->GetClass()->ImplementsInterface(UC_ExecutionRequester::StaticClass()))
	{
		
		IC_ExecutionRequester::Execute_requestExecution(Breaker, this);
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

bool AC_EnemyCharacter::isGuard() const
{
	return Super::isGuard();
}

bool AC_EnemyCharacter::isCombat() const
{
	return m_bInCombat;
}

void AC_EnemyCharacter::onParryFinished()
{
	finishAction(0.f);

	setActionState(E_ActionState::Free);
	setCombatState(E_CombatState::Idle);

	m_nextActionTime = GetWorld()->GetTimeSeconds() + 0.3f;
}

bool AC_EnemyCharacter::isBoss() const
{
	return m_eEnemyTier == E_EnemyTier::MiniBoss || m_eEnemyTier == E_EnemyTier::Boss;
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

void AC_EnemyCharacter::takeDamage_Implementation(float Damage, float PostureDamage, AActor* pAttacker)
{
	Super::takeDamage_Implementation(Damage, PostureDamage, pAttacker);

	if (UC_DetectComponent* pDetect = FindComponentByClass<UC_DetectComponent>())
	{
		pDetect->forceDetect(pAttacker);
	}
}

void AC_EnemyCharacter::onParried_Implementation(AActor* ParryOwner)
{
	Super::onParried_Implementation(ParryOwner);

}
