// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExecutionComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Sturcts/FS_ExecutionAnim.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "ProjectARPG/Interface/C_ExecutionTarget.h"

// Sets default values for this component's properties
UC_ExecutionComponent::UC_ExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

}

void UC_ExecutionComponent::BeginPlay()
{
	Super::BeginPlay();
	m_OwnerPlayer = Cast<AC_PlayerCharacter>(GetOwner());
}

void UC_ExecutionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_OwnerPlayer)
		return;

	if (m_OwnerPlayer->getCombatState() == E_CombatState::Executing)
		return;

	updateExecutionTarget();
}

void UC_ExecutionComponent::updateExecutionTarget()
{
	if (isValidCurrentTarget())
		return;

	findNewExecutionTarget();
}

bool UC_ExecutionComponent::isValidCurrentTarget() const
{
	if (!m_CurrentTargetActor.IsValid() || !m_CurrentTarget)
		return false;

	if (m_CurrentExecutionType == E_ExecutionType::None)
		return false;

	return true;
}

void UC_ExecutionComponent::findNewExecutionTarget()
{
	USphereComponent* ExecSphere = m_OwnerPlayer->getExecutionSphere();
	if (!ExecSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("[EXEC] No ExecutionSphere"));
		return;
	}
		

	TArray<AActor*> Overlaps;
	ExecSphere->GetOverlappingActors(Overlaps);

	UE_LOG(LogTemp, Warning,
		TEXT("[EXEC] Overlap count = %d"),
		Overlaps.Num());

	for (AActor* Act : Overlaps)
	{
		if (!Act->Implements<UC_ExecutionTarget>())
			continue;

		IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Act);
		if (!Target)
			continue;

		const bool bCan = Target->canBeExecuted(E_ExecutionType::PostureBreak);

		UE_LOG(LogTemp, Warning,
			TEXT("[EXEC] Check %s canBeExecuted = %d"),
			*Act->GetName(), bCan);

		if (bCan)
		{
			setCurrentExecutableTarget(
				Act,
				Target,
				E_ExecutionType::PostureBreak);

			Target->setExecutionHintVisible(true);

			UE_LOG(LogTemp, Warning,
				TEXT("[EXEC] Target SET = %s"),
				*Act->GetName());

			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[EXEC] No valid target"));
	clearCurrentTarget();
}

void UC_ExecutionComponent::setCurrentExecutableTarget(
	AActor* NewActor,
	IC_ExecutionTarget* NewTarget,
	E_ExecutionType Type)
{
	clearCurrentTarget();

	m_CurrentTargetActor = NewActor;
	m_CurrentTarget.SetObject(NewActor);
	m_CurrentTarget.SetInterface(NewTarget);
	m_CurrentExecutionType = Type;
}

void UC_ExecutionComponent::clearCurrentTarget()
{
	if (m_CurrentTarget)
		m_CurrentTarget->setExecutionHintVisible(false);

	m_CurrentTargetActor = nullptr;
	m_CurrentTarget = nullptr;
	m_CurrentExecutionType = E_ExecutionType::None;
}

bool UC_ExecutionComponent::tryExecuteCurrentTarget()
{
	if (!isValidCurrentTarget())
		return false;

	performExecution(
		m_OwnerPlayer,
		Cast<APawn>(m_CurrentTargetActor.Get()),
		m_CurrentExecutionType);

	return true;
}

void UC_ExecutionComponent::performExecution(APawn* Instigator, APawn* Victim, E_ExecutionType Type)
{
	if (!Instigator || !Victim)
		return;

	AC_PlayerCharacter* Player = Cast<AC_PlayerCharacter>(Instigator);
	IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Victim);

	if (!Player || !Target)
		return;

	Player->DisableInput(nullptr);
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->setCombatState(E_CombatState::Executing);

	Player->playPlayerExecutionMontage(Type);
	Target->onExecutionStarted(Player, Type);
}

bool UC_ExecutionComponent::canStartExecution() const
{
	const bool bValid = isValidCurrentTarget();

	UE_LOG(LogTemp, Warning,
		TEXT("[EXEC][Comp] canStartExecution = %d Type=%d"),
		bValid,
		(int)m_CurrentExecutionType);

	return bValid;
}

E_ExecutionType UC_ExecutionComponent::getCurrentExecutionType() const
{
	return m_CurrentExecutionType;
}

