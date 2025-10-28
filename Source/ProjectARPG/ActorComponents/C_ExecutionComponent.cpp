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

	pAttacker->setPlayerActionState(E_PlayerActionState::Executing);
	pAttacker->DisableInput(nullptr);
	pAttacker->GetCharacterMovement()->StopMovementImmediately();

	pEnemy->setCanBeExecuted(false);
	pEnemy->GetCharacterMovement()->DisableMovement();

	FVector vDir = (pEnemy->GetActorLocation() - pAttacker->GetActorLocation()).GetSafeNormal();
	FVector vTarget = pEnemy->GetActorLocation() - vDir * 150.f;

	pAttacker->SetActorLocation(vTarget);
	pAttacker->SetActorRotation(vDir.Rotation());

	if (m_pAttackerMontage && m_pEnemyMontage)
	{
		UAnimInstance* pAttackerAnim = pAttacker->GetMesh()->GetAnimInstance();
		UAnimInstance* pEnemyAnim = pEnemy->GetMesh()->GetAnimInstance();
		if (pAttackerAnim && pEnemyAnim)
		{
			pAttackerAnim->Montage_Play(m_pAttackerMontage);
			pEnemyAnim->Montage_Play(m_pEnemyMontage);

			UE_LOG(LogTemp, Error, TEXT("Execution Montage!!!!!!"));
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

