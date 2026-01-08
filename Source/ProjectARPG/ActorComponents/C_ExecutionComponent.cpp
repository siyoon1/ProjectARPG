// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExecutionComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Sturcts/FS_ExecutionAnim.h"
#include "AIController.h"
#include "BrainComponent.h"

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
	if (!m_pOwnerPlayer)
		return;

	if (m_pOwnerPlayer->getCombatState() == E_CombatState::Executing)
		return;

	USphereComponent* ExecSphere = m_pOwnerPlayer->getExecutionSphere();
	if (!ExecSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("ExecutionSphere is NULL"));
		return;
	}

	if (isValidCurrentTarget())
		return;




	findNewExecutionTarget();
}

void UC_ExecutionComponent::setCurrentExecutableTarget(AC_EnemyCharacter* pNewTarget, E_ExecutionType eType)
{
	if (m_pCurrentExecutableTarget == pNewTarget &&
		m_eCurrentExecutionType == eType)
		return;

	if (m_pCurrentExecutableTarget)
		m_pCurrentExecutableTarget->showExecutionVFX(false);

	m_pCurrentExecutableTarget = pNewTarget;
	m_eCurrentExecutionType = eType;

	if (m_pCurrentExecutableTarget && eType != E_ExecutionType::None)
		m_pCurrentExecutableTarget->showExecutionVFX(true);
}

bool UC_ExecutionComponent::isValidCurrentTarget() const
{
	if (!m_pCurrentExecutableTarget)
		return false;

	switch (m_eCurrentExecutionType)
	{
	case E_ExecutionType::Stealth:
		return canStealthExecute(m_pCurrentExecutableTarget);

	case E_ExecutionType::PostureBreak:
		return m_pCurrentExecutableTarget->canBeExecuted();

	default:
		return false;
	}
}

void UC_ExecutionComponent::findNewExecutionTarget()
{
	if (!m_pOwnerPlayer || !m_pOwnerPlayer->getExecutionSphere())
		return;

	TArray<AActor*> Overlaps{};
	m_pOwnerPlayer->getExecutionSphere()->GetOverlappingActors(
		Overlaps,
		AC_EnemyCharacter::StaticClass()
	);

	for (AActor* Act : Overlaps)
	{
		if (AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(Act))
		{
			if (canStealthExecute(Enemy))
			{
				setCurrentExecutableTarget(Enemy, E_ExecutionType::Stealth);
				return;
			}

			if (Enemy->canBeExecuted())
			{
				setCurrentExecutableTarget(Enemy, E_ExecutionType::PostureBreak);
				return;
			}
		}
	}

	// 못 찾았으면 해제
	clearCurrentTarget();
}

void UC_ExecutionComponent::clearCurrentTarget()
{
	setCurrentExecutableTarget(nullptr, E_ExecutionType::None);
}

bool UC_ExecutionComponent::isInStealthRange(AC_EnemyCharacter* pEnemy) const
{
	FVector BackPos =
		pEnemy->GetActorLocation() -
		pEnemy->GetActorForwardVector() * 80.f;

	float Dist =
		FVector::Dist(m_pOwnerPlayer->GetActorLocation(), BackPos);

	return Dist < 150.f;
}

bool UC_ExecutionComponent::isBehindTarget(AC_EnemyCharacter* pEnemy) const
{
	FVector EnemyToPlayer =
		(m_pOwnerPlayer->GetActorLocation() - pEnemy->GetActorLocation()).GetSafeNormal();

	float Dot =
		FVector::DotProduct(pEnemy->GetActorForwardVector(), EnemyToPlayer);

	return Dot < -0.5f;
}

bool UC_ExecutionComponent::canStealthExecute(AC_EnemyCharacter* pEnemy) const
{

	if (!pEnemy || !m_pOwnerPlayer)
		return false;

	if (pEnemy->isAnawareOfPlayer())
		return false;

	if (!isBehindTarget(pEnemy))
		return false;

	return isInStealthRange(pEnemy);

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

	if (!m_pExecutionAnimsTable)
		return;

	m_ExecutionMontages.Empty();

	static const FString Context(TEXT("ExecutionAnims Load"));
	TArray<FS_ExecutionAnim*> Rows;
	m_pExecutionAnimsTable->GetAllRows(Context, Rows);


	for (FS_ExecutionAnim* Row : Rows)
	{
		if (Row)
		{
			m_ExecutionMontages.Add(*Row);
		}
	}
	
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
	pEnemy->setCombatState(E_CombatState::Executing);

	pAttacker->DisableInput(nullptr);
	pAttacker->GetCharacterMovement()->StopMovementImmediately();

	pEnemy->setCanBeExecuted(false);
	pEnemy->GetCharacterMovement()->DisableMovement();

	if (AAIController* AICon = Cast<AAIController>(pEnemy->GetController()))
	{
		AICon->StopMovement();
		AICon->BrainComponent->StopLogic(TEXT("Executed"));
	}

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
			const FS_ExecutionAnim& sExecutionPair = m_ExecutionMontages[nIndex];

			if (UAnimInstance* AttackerAnim = pAttacker->GetMesh()->GetAnimInstance())
				if (UAnimInstance* EnemyAnim = pEnemy->GetMesh()->GetAnimInstance())
				{
					pAttacker->setCombatState(E_CombatState::Executing);
					AttackerAnim->Montage_Play(sExecutionPair.AttackerExecutionMontage);
					EnemyAnim->Montage_Play(sExecutionPair.VictimExecutionMontage);

					if (APlayerController* PC = Cast<APlayerController>(pAttacker->GetController()))
						if (AC_PlayerCameraManager* PCM =
							Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager))
						{
							PCM->executionEffect(
								sExecutionPair.AttackerExecutionMontage->GetPlayLength());
						}

					FOnMontageEnded OnEnd;
					OnEnd.BindUObject(
						this,
						&UC_ExecutionComponent::onExecutionFinished,
						pEnemy);

					AttackerAnim->Montage_SetEndDelegate(
						OnEnd,
						sExecutionPair.AttackerExecutionMontage);
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

	if (!m_pOwnerPlayer)
		return;

	if (m_pOwnerPlayer->getCombatState() == E_CombatState::Executing)
		return;

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
	if (!isValidCurrentTarget())
		return false;

	triggerExecution(m_pCurrentExecutableTarget, m_eCurrentExecutionType);
	return true;
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

	pVictim->onExecuted();

	if (AC_EnemyController* AICon = Cast<AC_EnemyController>(pVictim->GetController()))
	{
		AICon->executionFinishied(pOwner);
	}

	pOwner->EnableInput(nullptr);
	pOwner->setCombatState(E_CombatState::Idle);

	
}

