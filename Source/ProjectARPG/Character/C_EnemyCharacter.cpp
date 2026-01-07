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

		if (fDist > pData->fRange)
			continue;

		OutCandidates.Add(Row.Key);
	}

	
}

FName AC_EnemyCharacter::selectAttack(const TArray<FName>& Candidates) const
{
	if (Candidates.Num() == 0)
		return NAME_None;


	const FS_EnemyCombatProfile& profile = m_CurrentCombatProfile;

	TArray<float> fWeights{};
	float fTotal = 0.f;

	for (FName Row : Candidates)
	{
		const FS_AttackData* pData = getAttackData(Row);

		UE_LOG(LogTemp, Warning, TEXT("Row=%s pData=%s"),
			*Row.ToString(),
			pData ? TEXT("OK") : TEXT("NULL"));

		if (!pData)
		{
			fWeights.Add(0.f);
			continue;
		}

		float fWeight = 1.f;

		switch (pData->eProperty)
		{
		case E_AttackProperty::Thrust:
			fWeight = profile.fThrustWeight;
			break;
		case E_AttackProperty::Heavy:
			fWeight = profile.fHeavyWeight;
			break;
		default:
			fWeight = 1.f;
			break;
		}
	

		fWeights.Add(fWeight);
		fTotal += fWeight;
	}

	if (fTotal <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("TotalWeight == 0, fallback"));
		return Candidates[0];
	}

	float fPick = FMath::FRandRange(0.f, fTotal);
	float fAcc = 0.f;

	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		fAcc += fWeights[i];
		if (fPick <= fAcc)
			return Candidates[i];
	}

	return Candidates.Last();
}

bool AC_EnemyCharacter::decideNextAttack(float fDist, FName& OutRow)
{
	TArray<FName> Candidates{};
	getAttackCandidates(fDist, Candidates);

	UE_LOG(LogTemp, Warning, TEXT("Candidates Num = %d"), Candidates.Num());

	if (Candidates.Num() == 0)
		return false;

	FName SelectedRow = selectAttack(Candidates);
	if (SelectedRow.IsNone())
		return false;

	OutRow = SelectedRow;

	return true;
}

bool AC_EnemyCharacter::attack(const FS_AttackData* pAttackData)
{
	if (!pAttackData || m_bIsExecutingAction)
		return false;

	m_bIsExecutingAction = true;
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

void AC_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_EnemyCharacter::playStepBack()
{
	m_bIsExecutingAction = true;

	if (UC_EnemyAnim* pAnim = Cast< UC_EnemyAnim>(GetMesh()->GetAnimInstance()))
	{
		pAnim->playStepBackMontage();
	}
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
	return m_pCurrentAttackData ? m_pCurrentAttackData->fRange : m_CurrentCombatProfile.fPreferredRange;
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

void AC_EnemyCharacter::endGuard()
{
	setGuard(false);
	m_bIsExecutingAction = false;

	const auto* profile = m_CombatProfiles.Find(m_eEnemyTier);

	m_nextActionTime = GetWorld()->GetTimeSeconds() + (profile ? profile->fActionInterval : 0.25f);
}

void AC_EnemyCharacter::endAttack()
{
	m_bIsExecutingAction = false;

	const auto* profile = m_CombatProfiles.Find(m_eEnemyTier);

	m_nextActionTime = GetWorld()->GetTimeSeconds() + (profile ? profile->fActionInterval : 0.25f);

	m_onAttackFinished.Broadcast();
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
