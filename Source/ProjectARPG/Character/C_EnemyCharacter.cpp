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
#include "ProjectARPG/ActorComponents/C_LockOnComponent.h"
#include "Components/BillboardComponent.h"
#include "ProjectARPG/AI/C_NormalCombatDecision.h"
#include "ProjectARPG/AI/C_BossCombatDecision.h"


AC_EnemyCharacter::AC_EnemyCharacter()
{
	m_AIAttackComp = CreateDefaultSubobject<UC_AIAttackComponent>(TEXT("EnemyAttackComp"));

	m_wHpBarCom = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpBarComp"));
	m_wHpBarCom->SetupAttachment(RootComponent);
	m_wHpBarCom->SetWidgetSpace(EWidgetSpace::Screen);

	m_wLockOnCom = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnComp"));
	m_wLockOnCom->SetupAttachment(RootComponent);
	m_wLockOnCom->SetWidgetSpace(EWidgetSpace::Screen);
}

void AC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_NormalDecision = NewObject<UC_NormalCombatDecision>(this);
	m_BossDecision = NewObject<UC_BossCombatDecision>(this);

	m_CurrentLifeNodes = m_MaxLifeNodes;

	m_DetectCom = GetComponentByClass<UC_DetectComponent>();

	m_pPlayer = Cast< AC_CombatCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (m_wLockOnCom)
		m_wLockOnCom->SetVisibility(false);

	if (m_wHpBarCom)
		m_wHpBarCom->SetVisibility(false);

	setExecutionHintVisible(false);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			if (UC_LockOnComponent* LockOn =
				PlayerPawn->FindComponentByClass<UC_LockOnComponent>())
			{
				LockOn->OnLockOnStarted.AddDynamic(
					this, &AC_EnemyCharacter::onLockOnStarted);

				LockOn->OnLockOnEnded.AddDynamic(
					this, &AC_EnemyCharacter::onLockOnEnded);
			}
		}
	}



}

void AC_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isBoss() && m_bIsGuarding)
	{
		const float Elapsed =
			GetWorld()->GetTimeSeconds() - m_fGuardStartTime;

		if (Elapsed > m_CombatConfig.GuardMaxTime + 0.5f)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[GuardFailSafe] Force end guard : %s"), *GetName());

			endGuard();
			m_EnemyActionState = E_EnemyActionState::Idle;
		}
	}

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
	if (isGuard())
	{
		finishAction(0.2f);
		return false;
	}

	if (!Super::startGuard())
		return false;

	m_fGuardStartTime = GetWorld()->GetTimeSeconds();
	return true;
}

void AC_EnemyCharacter::endGuard()
{
	if (!isGuard())
		return;

	Super::endGuard();

	m_EnemyActionState = E_EnemyActionState::Idle;

	if (m_bCounterWindowOpen && isBoss())
	{
		m_bCounterWindowOpen = false;

		if (m_AIAttackComp)
		{
			const float Dist = getDistToTarget();
			if (m_AIAttackComp->tryExecuteAttack(Dist))
			{
				beginAction();
				return;
			}
		}
	}

	const float Cooldown = m_CombatConfig.ActionInterval;
	finishAction(Cooldown);

	m_LastAction = E_EnemyActionType::Guard;
	m_CurrentAction = E_EnemyActionType::None;
}

bool AC_EnemyCharacter::canReleaseGuard() const
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - m_fGuardStartTime;

	if (Elapsed < m_CombatConfig.GuardMinTime)
		return false;

	if (Elapsed >= m_CombatConfig.GuardMaxTime)
		return true;

	if (m_PlayerAttackChain >= 2)
		return true;

	// 카운터 윈도우 열리면 바로 반격
	if (m_bCounterWindowOpen)
		return true;

	const float GuardHoldTime = 0.6f;
	return GetWorld()->GetTimeSeconds() - m_fGuardStartTime >= GuardHoldTime;
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

	const float Dist = getDistToTarget();
	const float MaxSafeDist =
		m_CombatTendency.PreferredRange * 1.1f;

	if (Dist >= MaxSafeDist)
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
	const float Cooldown = m_CombatConfig.ActionInterval;
	finishAction(Cooldown);

	m_onStepBackFinished.Broadcast();
}


void AC_EnemyCharacter::endAttack()
{
	const float Cooldown = m_CombatConfig.ActionInterval;
	finishAction(Cooldown);

	m_LastAction = E_EnemyActionType::Attack;
	m_CurrentAction = E_EnemyActionType::None;

	m_onAttackFinished.Broadcast();
}

void AC_EnemyCharacter::showHpBar(bool bShow)
{
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

	if (m_eEnemyTier >= E_EnemyTier::MiniBoss)
	{
		m_onBossStateChanged.Broadcast(this, true);
	}
}

void AC_EnemyCharacter::onCombatEnded()
{
	m_bInCombat = false;

	setInCombat(false);
	showHpBar(false);

	if (m_eEnemyTier >= E_EnemyTier::MiniBoss)
	{
		m_onBossStateChanged.Broadcast(this, false);
	}

	m_DetectCom->resetDetect();
}

float AC_EnemyCharacter::getNextActionTime() const
{
	return m_nextActionTime;
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
	if (isDead())
		return;

	if (m_CurrentLifeNodes > 0 && m_eEnemyTier >= E_EnemyTier::MiniBoss)
	{
		m_CurrentLifeNodes--;

		m_OnLifeNodeChanged.Broadcast(m_CurrentLifeNodes, m_MaxLifeNodes);

		if (m_CurrentLifeNodes <= 0)
		{
			onDeath();
			return;
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}

		if (AAIController* AICon = Cast<AAIController>(GetController()))
		{
			if (AICon->BrainComponent)
			{
				AICon->BrainComponent->RestartLogic();
			}
		}

		m_bIsPostureBroken = false;

		GetWorldTimerManager().ClearTimer(m_timerHandle_PostureBroken);


		enterCombatMode(E_CombatMode::None, E_ActionState::Free);

		if (m_StatComp)
			m_StatComp->initStat();

		if (isGuard())
		{
			Super::endGuard();
		}

		m_EnemyActionState = E_EnemyActionState::Idle;

		m_DetectCom->forceDetect(ExecutionInstigator);
	}
	else
	{
		onDeath();
	}

	
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

float AC_EnemyCharacter::getGuardPostureMultiplier() const
{
	return isBoss() ? 1.2f : 0.7f;
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

	setActionState(E_ActionState::Dead);
	showExecutionVFX(false);
	m_wHpBarCom->DestroyComponent();


}

void AC_EnemyCharacter::takeDamage_Implementation(float Damage, float PostureDamage, E_HitResult HitResult, AActor* pAttacker)
{
	Super::takeDamage_Implementation(Damage, PostureDamage, HitResult, pAttacker);

	if (UC_DetectComponent* pDetect = FindComponentByClass<UC_DetectComponent>())
	{
		pDetect->forceDetect(pAttacker);
	}
}

void AC_EnemyCharacter::onParried_Implementation(AActor* ParryOwner)
{
	Super::onParried_Implementation(ParryOwner);

}

void AC_EnemyCharacter::onLockOnStarted(AActor* Target)
{
	if (Target != this)
		return;

	if (m_wLockOnCom)
		m_wLockOnCom->SetVisibility(true);
}

void AC_EnemyCharacter::onLockOnEnded(AActor* Target)
{
	if (Target != this)
		return;

	if (m_wLockOnCom)
		m_wLockOnCom->SetVisibility(false);
}

int32 AC_EnemyCharacter::getPlayerAttackChain() const
{
	return m_PlayerAttackChain;
}

const bool AC_EnemyCharacter::isCounterWindowOpen() const
{
	return m_bCounterWindowOpen;
}

void AC_EnemyCharacter::openCounterWindow()
{
	if (m_bCounterWindowOpen)
		return;

	m_bCounterWindowOpen = true;

	if (isBoss() && isGuard())
	{
		endGuard();
		return;
	}

	GetWorldTimerManager().SetTimer(
		m_CounterWindowTimer,
		this,
		&AC_EnemyCharacter::closeCounterWindow,
		m_CounterWindowTime,
		false
	);
}

void AC_EnemyCharacter::closeCounterWindow()
{
	m_bCounterWindowOpen = false;
}
