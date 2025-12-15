// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExecutionComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UC_ExecutionComponent::UC_ExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

}

void UC_ExecutionComponent::updateExecutionTarget()
{
	TArray<AActor*> Candidates{};

	if (m_pOwnerPlayer)
		m_pOwnerPlayer->getExecutionSphere()->GetOverlappingActors
		(
			Candidates,
			AC_EnemyCharacter::StaticClass()
		);

	AC_EnemyCharacter* pBestTarget = nullptr;
	E_ExecutionType eBestType = E_ExecutionType::None;

	for (AActor* Act : Candidates)
	{
		auto pEnemy = Cast<AC_EnemyCharacter>(Act);

		if (!pEnemy)
			continue;

		if (pEnemy->canBeExecuted())
		{
			pBestTarget = pEnemy;
			eBestType = E_ExecutionType::PostureBreak;
			break;
		}

		if (canStealthExecute(pEnemy))
		{
			pBestTarget = pEnemy;
			eBestType = E_ExecutionType::Stealth;
			break;
		}


		pBestTarget = pEnemy;
		break;
	}

	setCurrentExecutableTarget(pBestTarget, eBestType);
}

void UC_ExecutionComponent::setCurrentExecutableTarget(AC_EnemyCharacter* pNewTarget, E_ExecutionType eType)
{
	if (m_pCurrentExecutableTarget == pNewTarget && m_eCurrentExecutionType == eType)
		return;

	if (m_pCurrentExecutableTarget)
		m_pCurrentExecutableTarget->showExecutionVFX(false);

	m_pCurrentExecutableTarget = pNewTarget;
	m_eCurrentExecutionType = eType;

	if (m_pCurrentExecutableTarget)
		m_pCurrentExecutableTarget->showExecutionVFX(true);
}

bool UC_ExecutionComponent::isBehindTarget(AC_EnemyCharacter* pEnemy) const
{
	if (!pEnemy || !m_pOwnerPlayer)
		return false;

	const FVector vEnemyForward = pEnemy->GetActorForwardVector();
	const FVector vToPlayer = (m_pOwnerPlayer->GetActorLocation() - pEnemy->GetActorLocation()).GetSafeNormal();

	const float fDot = FVector::DotProduct(vEnemyForward, vToPlayer);



	return fDot < -0.6f;
}

bool UC_ExecutionComponent::canStealthExecute(AC_EnemyCharacter* pEnemy) const
{
	if (!pEnemy || !m_pOwnerPlayer)
		return false;

	if (m_pOwnerPlayer->getCombatState() != E_CombatState::Crouch)
		return false;

	if (!pEnemy->isUnawareOfPlayer())
		return false;

	if (!isBehindTarget(pEnemy))
		return false;

	const float fDist = FVector::Dist(m_pOwnerPlayer->GetActorLocation(), pEnemy->GetActorLocation());




	return fDist < 130.f;
}


// Called when the game starts
void UC_ExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	m_pOwnerPlayer = Cast<AC_PlayerCharacter>(GetOwner());

	// ...
	AC_EnemyCharacter* pChar = Cast<AC_EnemyCharacter>(GetOwner());

	if (pChar)
		pChar->showExecutionVFX(false);
	
}

void UC_ExecutionComponent::performExecution(APawn* pInstigator, APawn* pVictim, E_ExecutionType eType)
{
	if (!pInstigator || !pVictim)
		return;

	AC_PlayerCharacter* pAttacker = Cast<AC_PlayerCharacter>(pInstigator);
	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pVictim);

	if (!pAttacker || !pEnemy)
		return;

	UE_LOG(LogTemp, Error, TEXT("pAttacker => %s  pEnemy => %s"),
		*pAttacker->GetName(), *pEnemy->GetName());

	pAttacker->setCombatState(E_CombatState::Executing);
	pAttacker->DisableInput(nullptr);
	pAttacker->GetCharacterMovement()->StopMovementImmediately();

	pEnemy->setCanBeExecuted(false);
	pEnemy->GetCharacterMovement()->DisableMovement();

	switch (eType)
	{
	case E_ExecutionType::Stealth:
	case E_ExecutionType::PostureBreak:
	{
		FVector vDirToEnemy =
			(pEnemy->GetActorLocation() - pAttacker->GetActorLocation()).GetSafeNormal();
		FVector vTarget =
			pEnemy->GetActorLocation() - vDirToEnemy * 180.f;

		pAttacker->SetActorLocation(vTarget);
		pAttacker->SetActorRotation(vDirToEnemy.Rotation());
		pEnemy->SetActorRotation((-vDirToEnemy).Rotation());

		if (m_ExecutionMontages.Num() > 0)
		{
			int32 nIndex = FMath::RandRange(0, m_ExecutionMontages.Num() - 1);
			const FS_ExecutionMontagePair& sExecutionPair = m_ExecutionMontages[nIndex];

			if (UAnimInstance* AttackerAnim = pAttacker->GetMesh()->GetAnimInstance())
				if (UAnimInstance* EnemyAnim = pEnemy->GetMesh()->GetAnimInstance())
				{
					AttackerAnim->Montage_Play(sExecutionPair.sAttackerMontage);
					EnemyAnim->Montage_Play(sExecutionPair.sEnemyMontage);

					if (APlayerController* PC = Cast<APlayerController>(pAttacker->GetController()))
						if (AC_PlayerCameraManager* PCM =
							Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager))
						{
							PCM->executionEffect(
								sExecutionPair.sAttackerMontage->GetPlayLength());
						}

					FOnMontageEnded OnEnd;
					OnEnd.BindUObject(
						this,
						&UC_ExecutionComponent::onExecutionFinished,
						pEnemy);

					AttackerAnim->Montage_SetEndDelegate(
						OnEnd,
						sExecutionPair.sAttackerMontage);
				}
		}
		break;
	}
	}
	
}


// Called every frame
void UC_ExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	updateExecutionTarget();
}

void UC_ExecutionComponent::onBecomeExecutable(APawn* pVictim)
{
	AC_EnemyCharacter* pChar = Cast<AC_EnemyCharacter>(GetOwner());

	if (pChar)
		pChar->showExecutionVFX(true);
		
}

void UC_ExecutionComponent::triggerExecution(APawn* pVictim, E_ExecutionType eType)
{
	if (!pVictim)
		return;

	AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(GetOwner());

	if (!pOwner)
		return;


	performExecution(pOwner, pVictim, eType);

}

bool UC_ExecutionComponent::tryExecuteCurrentTarget()
{
	if (!m_pCurrentExecutableTarget || !m_pOwnerPlayer)
		return false;

	UE_LOG(LogTemp, Warning,
		TEXT("TryExec | Type=%d Crouch=%d Unaware=%d"),
		(int)m_eCurrentExecutionType,
		m_pOwnerPlayer->bIsCrouched,
		m_pCurrentExecutableTarget->isUnawareOfPlayer());


	switch (m_eCurrentExecutionType)
	{
	case E_ExecutionType::Stealth:
	{
		if (m_pOwnerPlayer->getCombatState() != E_CombatState::Crouch)
			return false;

		if (!m_pCurrentExecutableTarget->isUnawareOfPlayer())
			return false;

		triggerExecution(m_pCurrentExecutableTarget, E_ExecutionType::Stealth);
		return true;
	}
		

	case E_ExecutionType::PostureBreak:
	{

		triggerExecution(m_pCurrentExecutableTarget, E_ExecutionType::PostureBreak);
		return true;
	}
	}

	
	return false;
}

void UC_ExecutionComponent::playStunMontage()
{
	AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(GetOwner());

	if (!pOwner)
		return;

	if (UAnimInstance* pAnim = Cast<UAnimInstance>(pOwner->GetMesh()->GetAnimInstance()))
	{
		pAnim->Montage_Play(m_pStunMontage);
	}
}

void UC_ExecutionComponent::onExecutionFinished(UAnimMontage* Montage, bool bInterrupted, AC_EnemyCharacter* pVictim)
{
	if (!pVictim)
		return;

	AC_PlayerCharacter* pOwner = Cast<AC_PlayerCharacter>(GetOwner());
	if (!pOwner)
		return;

	pVictim->setCombatState(E_CombatState::Die);

	pOwner->EnableInput(nullptr);
	pOwner->setCombatState(E_CombatState::Idle);

}

