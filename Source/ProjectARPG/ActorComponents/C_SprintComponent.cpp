// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SprintComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Camera/C_PlayerCameraManager.h"
#include "ProjectARPG/Animation/C_PlayerAnim.h"



// Sets default values for this component's properties
UC_SprintComponent::UC_SprintComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



// Called when the game starts
void UC_SprintComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UC_SprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_SprintComponent::init(AC_PlayerCharacter* InOwner)
{
	m_Owner = InOwner;

	if (!m_Owner)
		return;

	m_DefaultSpeed = m_Owner->GetCharacterMovement()->MaxWalkSpeed;

	if (APlayerController* PC = Cast<APlayerController>(m_Owner->GetController()))
	{
		m_CameraMgr = Cast<AC_PlayerCameraManager>(PC->PlayerCameraManager);
	}
}

bool UC_SprintComponent::canStartSprint() const
{
	if (!m_Owner)
		return false;

	if (m_bIsSprinting)
		return false;

	if (m_Owner->getActionState() != E_ActionState::Free)
		return false;

	if (m_Owner->isCrouch())
		return false;

	return true;
}

bool UC_SprintComponent::startSprint()
{
	if (!canStartSprint())
		return false;

	UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(m_Owner->GetMesh()->GetAnimInstance());
	if (!pAnim)
		return false;

	m_bIsSprinting = true;

	m_Owner->setCombatState(E_CombatState::Sprinting);
	m_Owner->GetCharacterMovement()->MaxWalkSpeed = m_SprintSpeed;

	pAnim->playSprintStartMontage();

	if (m_CameraMgr)
	{
		m_CameraMgr->startSprintEffect();
	}

	const FVector LaunchVel = m_Owner->GetActorForwardVector() * m_SprintSpeed;
	m_Owner->LaunchCharacter(LaunchVel, true, false);

	return true;
}

void UC_SprintComponent::stopSprint(E_SprintEndReason Reason)
{
	if (!m_bIsSprinting)
		return;

	m_bIsSprinting = false;

	restoreMovement();

	if (m_CameraMgr)
	{
		m_CameraMgr->stopSprintEffect();
	}

	m_Owner->setCombatState(E_CombatState::Idle);
}

void UC_SprintComponent::forceStop()
{
	stopSprint(E_SprintEndReason::Forced);
}

void UC_SprintComponent::restoreMovement()
{
	if (!m_Owner)
		return;

	m_Owner->GetCharacterMovement()->MaxWalkSpeed = m_DefaultSpeed;
}


