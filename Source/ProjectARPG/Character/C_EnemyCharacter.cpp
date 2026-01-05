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

	m_CurrentCombatProfile = *profile;

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			BB->SetValueAsFloat(AC_EnemyController::DistKey, m_CurrentCombatProfile.fAttackRange);
			BB->SetValueAsFloat(AC_EnemyController::AttackProbKey, m_CurrentCombatProfile.fAttackProbability);
			BB->SetValueAsFloat(AC_EnemyController::GuardProbKey, m_CurrentCombatProfile.fGuardProbability);
		}
	}
}

void AC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_DetectCom = GetComponentByClass<UC_DetectComponent>();

	m_pPlayer = Cast< AC_CombatCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	applyCombatProfile();

	showHpBar(false);
}

E_EnemyAttackType AC_EnemyCharacter::decideAttackType() const
{
	const FS_EnemyCombatProfile& profile = m_CurrentCombatProfile;

	float fRan = FMath::FRand();

	if (fRan < profile.fThrustRatio)
		return E_EnemyAttackType::Thrust;

	if (fRan < 0.8f)
		return E_EnemyAttackType::Heavy;

	return E_EnemyAttackType::Light;
}

bool AC_EnemyCharacter::isPlayerAttacking() const
{
	if (!m_pPlayer)
		return false;

	if (m_pPlayer->getCombatState() != E_CombatState::Attacking)
		return false;

	float fDist = FVector::Dist(GetActorLocation(), m_pPlayer->GetActorLocation());
	if (fDist > m_fAttackRange)
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
}

void AC_EnemyCharacter::attack()
{
	if (m_bIsExecutingAction)
		return;

	m_bIsExecutingAction = true;

	m_eCurrentAttackType = decideAttackType();

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

bool AC_EnemyCharacter::isBoss() const
{
	return m_eEnemyTier == E_EnemyTier::MiniBoss || m_eEnemyTier == E_EnemyTier::Boss;
}

void AC_EnemyCharacter::onExecuted()
{
	Super::onExecuted();

	m_bIsExecutingAction = false;
	m_bInCombat = true;

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
