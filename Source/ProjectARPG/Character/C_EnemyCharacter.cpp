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


void AC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_DetectCom = GetComponentByClass<UC_DetectComponent>();

	m_pPlayer = Cast< AC_CombatCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	applyCombatProfile();

	showHpBar(false);
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

void AC_EnemyCharacter::getAttackCandidates(float fDist,TArray<FName>& OutCandidates) const
{
	OutCandidates.Empty();

	if (!m_pAttackDataTable)
		return;

	for (const auto& Row : m_pAttackDataTable->GetRowMap())
	{
		FS_AttackData* pData = m_pAttackDataTable->FindRow<FS_AttackData>(Row.Key, TEXT("getAttackCandidates"));

		if (!pData)
			continue;

		if (!canUseAttack(Row.Key))
			continue;

		OutCandidates.Add(Row.Key);
	}

	
}

FName AC_EnemyCharacter::selectAttack(const TArray<FName>& Candidates) const
{
	if (Candidates.Num() == 0)
		return NAME_None;

	const FS_EnemyCombatProfile& profile = m_CurrentCombatProfile;

	TArray<float> Weights;
	Weights.Reserve(Candidates.Num());

	float fTotalWeight = 0.f;

	for (FName Row : Candidates)
	{
		const FS_AttackData* Data = getAttackData(Row);
		if (!Data || !canUseAttack(Row))
		{
			Weights.Add(0.f);
			continue;
		}

		float Weight = Data->fBaseWeight;

		switch (Data->eProperty)
		{
		case E_AttackProperty::Thrust:
			Weight *= profile.fThrustWeight;
			break;
		case E_AttackProperty::Heavy:
			Weight *= profile.fHeavyWeight;
			break;
		}

		if (const FS_AttackRuntimeState* State = m_AttackStates.Find(Row))
		{
			const float Now = GetWorld()->GetTimeSeconds();

			if (Data->fMinReuseTime > 0.f)
			{
				const float Elapsed = Now - State->LastUsedTime;
				if (Elapsed < Data->fMinReuseTime)
				{
					const float CooldownRatio = Elapsed / Data->fMinReuseTime;
					Weight *= FMath::Clamp(CooldownRatio * 0.2f, 0.05f, 0.3f);
				}
			}

			Weight *= FMath::Clamp(1.f - State->Fatigue, 0.1f, 1.f);
		}

		const float Dist = getDistanceToTarget();
		const float RangeFactor = FMath::Clamp(
			1.f - FMath::Abs(Dist - Data->fIdealRange) / Data->fIdealRange,
			0.3f,
			1.f
		);

		Weight *= RangeFactor;

		Weights.Add(Weight);
		fTotalWeight += Weight;
	}

	if (fTotalWeight <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("TotalWeight == 0, fallback"));
		return Candidates[0];
	}

	const float Pick = FMath::FRandRange(0.f, fTotalWeight);
	float Acc = 0.f;

	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		Acc += Weights[i];
		if (Pick <= Acc)
			return Candidates[i];
	}

	return Candidates.Last();
}

bool AC_EnemyCharacter::canUseAttack(FName Row) const
{
	const FS_AttackData* Data = getAttackData(Row);
	if (!Data)
		return false;

	if (!m_pPlayer)
		return false;

	return true;
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

	UE_LOG(LogTemp, Warning,
		TEXT("[Enemy] canReleaseGuard elapsed=%.2f min=%.2f max=%.2f threatening=%d"),
		fElapsed,
		m_CurrentCombatProfile.fGuardMinTime,
		m_CurrentCombatProfile.fGuardMaxTime,
		isPlayerThreatening()
	);

	if (fElapsed < m_CurrentCombatProfile.fGuardMinTime)
		return false;

	if (fElapsed >= m_CurrentCombatProfile.fGuardMaxTime)
		return true;

	if (isPlayerThreatening())
		return false;

	return true;
}

void AC_EnemyCharacter::endGuard()
{
	setGuard(false);
	m_bIsGuarding = false;

	const float Cooldown = m_CurrentCombatProfile.fActionInterval;
	finishAction(Cooldown);
}

bool AC_EnemyCharacter::isAttackInRange(const FS_AttackData& Data, float fDistance) const
{
	if (fDistance < Data.fMinRange)
		return false;

	if (fDistance > Data.fMaxRange)
		return false;

	return true;
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

bool AC_EnemyCharacter::decideNextAttack(float fDist, FName& OutRow)
{
	TArray<FName> Candidates;
	getAttackCandidates(fDist, Candidates);

	if (Candidates.Num() == 0)
		return false; 

	OutRow = selectAttack(Candidates);
	return !OutRow.IsNone();
}

bool AC_EnemyCharacter::attack(const FS_AttackData* pAttackData)
{
	if (!pAttackData || !canDecideAction())
		return false;

	beginAction();
	m_pCurrentAttackData = pAttackData;

	if (m_pPlayer)
	{
		FVector dir = m_pPlayer->GetActorLocation() - GetActorLocation();
		dir.Z = 0;
		SetActorRotation(dir.Rotation());
	}

	if (UAnimInstance* pAnim = GetMesh()->GetAnimInstance())
	{
		if (UC_EnemyAnim* pEnemyAnim = Cast<UC_EnemyAnim>(pAnim))
		{
			pEnemyAnim->playAttackMontage(pAttackData->pMontage);
		}
	}

	return true;
}

bool AC_EnemyCharacter::isPlayerAttacking() const
{
	if (!m_pPlayer)
		return false;

	if (m_pPlayer->getCombatState() != E_CombatState::Attacking)
		return false;


	float fDist = FVector::Dist(GetActorLocation(), m_pPlayer->GetActorLocation());
	if (fDist > m_CurrentCombatProfile.fPreferredRange)
		return false;

	FVector vPlayerForward = m_pPlayer->GetActorForwardVector();
	FVector vToEnemy = (GetActorLocation() - m_pPlayer->GetActorLocation()).GetSafeNormal();

	float fDot = FVector::DotProduct(vPlayerForward, vToEnemy);

	if (fDot < 0.5f)
		return false;

	return true;
}

bool AC_EnemyCharacter::canConsiderAttack(float fDist) const
{
	return fDist <= getAttackMaxRange() * 1.1f;
}

float AC_EnemyCharacter::getDistanceToTarget() const
{
	if (!m_pPlayer)
		return 0.f;

	return FVector::Dist(GetActorLocation(), m_pPlayer->GetActorLocation());
}

void AC_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

bool AC_EnemyCharacter::isPlayerThreatening() const
{
	if (!m_pPlayer)
		return false;

	if (m_pPlayer->getCombatState() != E_CombatState::Attacking)
		return false;

	const float Dist =
		FVector::Dist(GetActorLocation(), m_pPlayer->GetActorLocation());

	if (Dist > m_CurrentCombatProfile.fPreferredRange * 1.1f)
		return false;

	const FVector ToEnemy =
		(GetActorLocation() - m_pPlayer->GetActorLocation()).GetSafeNormal();

	const float Dot =
		FVector::DotProduct(m_pPlayer->GetActorForwardVector(), ToEnemy);

	if (Dot < 0.6f)
		return false;



	return true;
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

float AC_EnemyCharacter::getAttackMinRange() const
{
	return m_pCurrentAttackData ? m_pCurrentAttackData->fMinRange : m_CurrentCombatProfile.fPreferredRange * 0.7f;
}

float AC_EnemyCharacter::getAttackMaxRange() const
{
	return m_pCurrentAttackData ? m_pCurrentAttackData->fMaxRange : m_CurrentCombatProfile.fPreferredRange;
}

float AC_EnemyCharacter::getAttackIdealRange() const
{
	return m_pCurrentAttackData ? m_pCurrentAttackData->fIdealRange : m_CurrentCombatProfile.fPreferredRange;
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

		if (m_pExecutionCom)
		{
			m_pExecutionCom->onBecomeExecutable(this);
			UE_LOG(LogTemp, Error, TEXT("ONBECOMEEXCUTABLE!!!!"));
		}


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
