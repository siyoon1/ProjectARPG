// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExecutionComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UC_ExecutionComponent::UC_ExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

}


// Called when the game starts
void UC_ExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	AC_EnemyCharacter* pChar = Cast<AC_EnemyCharacter>(GetOwner());

	if (pChar)
		pChar->showExecutionVFX(false);
	
}

void UC_ExecutionComponent::performExecution(APawn* pInstigator, APawn* pVictim)
{
	if (!pInstigator || !pVictim)
		return;

	AC_PlayerCharacter* pAttacker = Cast<AC_PlayerCharacter>(pInstigator);
	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(pVictim);

	if (!pAttacker || !pEnemy)
		return;

	


	pAttacker->setCombatState(E_CombatState::Executing);
	pAttacker->DisableInput(nullptr);
	pAttacker->GetCharacterMovement()->StopMovementImmediately();

	pEnemy->setCanBeExecuted(false);
	pEnemy->GetCharacterMovement()->DisableMovement();

	FVector vDirToEnemy = (pEnemy->GetActorLocation() - pAttacker->GetActorLocation()).GetSafeNormal();
	FVector vTarget = pEnemy->GetActorLocation() - vDirToEnemy * 180.f;

	pAttacker->SetActorLocation(vTarget);

	FRotator rPlayerRot = vDirToEnemy.Rotation();
	pAttacker->SetActorRotation(rPlayerRot);

	FRotator rEnemyRot = (-vDirToEnemy).Rotation();
	pEnemy->SetActorRotation(rEnemyRot);

	if (m_ExecutionMontages.Num() > 0)
	{
		int32 nIndex = FMath::RandRange(0, m_ExecutionMontages.Num() - 1);
		const FS_ExecutionMontagePair& sExecutionPair = m_ExecutionMontages[nIndex];

		UAnimInstance* pAttackerAnim = pAttacker->GetMesh()->GetAnimInstance();
		UAnimInstance* pEnemyAnim = pEnemy->GetMesh()->GetAnimInstance();

		if (pAttackerAnim && pEnemyAnim)
		{
			pAttackerAnim->Montage_Play(sExecutionPair.sAttackerMontage);
			pEnemyAnim->Montage_Play(sExecutionPair.sEnemyMontage);

			if (APlayerController* PC = Cast<APlayerController>(pAttacker->GetController()))
			{
				if (AC_PlayerCameraManager* PCM = Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager))
				{
					PCM->executionEffect(sExecutionPair.sAttackerMontage->GetPlayLength());
				}
			}

			UE_LOG(LogTemp, Error, TEXT("Execution Montage!!!!!!"));

			FOnMontageEnded onMontageEnd;
			onMontageEnd.BindUObject(this, &UC_ExecutionComponent::onExecutionFinished, pEnemy);
			pAttackerAnim->Montage_SetEndDelegate(onMontageEnd, sExecutionPair.sAttackerMontage);
		}

	}
}


// Called every frame
void UC_ExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_ExecutionComponent::onBecomeExecutable(APawn* pVictim)
{
	AC_EnemyCharacter* pChar = Cast<AC_EnemyCharacter>(GetOwner());

	if (pChar)
		pChar->showExecutionVFX(true);
		
}

void UC_ExecutionComponent::triggerExecution(APawn* pVictim)
{
	if (!pVictim)
		return;

	AC_CombatCharacter* pOwner = Cast<AC_CombatCharacter>(GetOwner());

	if (!pOwner)
		return;

	performExecution(pOwner, pVictim);
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

