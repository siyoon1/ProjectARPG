// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerExecutionComponent.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Camera/C_PlayerCameraManager.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Interface/C_ExecutionTarget.h"

// Sets default values for this component's properties
UC_PlayerExecutionComponent::UC_PlayerExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

}

void UC_PlayerExecutionComponent::BeginPlay()
{
	Super::BeginPlay();
	m_OwnerPlayer = Cast<AC_PlayerCharacter>(GetOwner());
}

void UC_PlayerExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_OwnerPlayer)
		return;

	if (m_OwnerPlayer->getCombatState() == E_CombatState::Executing)
		return;

	updateExecutionTarget();
}

void UC_PlayerExecutionComponent::updateExecutionTarget()
{
	if (isValidCurrentTarget())
		return;

	findNewExecutionTarget();
}

bool UC_PlayerExecutionComponent::isValidCurrentTarget() const
{
	if (!m_CurrentTargetActor.IsValid() || !m_CurrentTarget)
		return false;

	if (m_CurrentExecutionType == E_ExecutionType::None)
		return false;

	return true;
}

void UC_PlayerExecutionComponent::findNewExecutionTarget()
{
	USphereComponent* ExecSphere = m_OwnerPlayer->getExecutionSphere();
	if (!ExecSphere)
	{
		return;
	}


	TArray<AActor*> Overlaps;
	ExecSphere->GetOverlappingActors(Overlaps);

	for (AActor* Act : Overlaps)
	{
		if (!Act->Implements<UC_ExecutionTarget>())
			continue;

		IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Act);
		if (!Target)
			continue;

		const bool bCan = Target->canBeExecuted(E_ExecutionType::PostureBreak);

		if (bCan)
		{
			setCurrentExecutableTarget(
				Act,
				Target,
				E_ExecutionType::PostureBreak);

			Target->setExecutionHintVisible(true);

			return;
		}
	}

	clearCurrentTarget();
}

void UC_PlayerExecutionComponent::setCurrentExecutableTarget(AActor* NewActor, IC_ExecutionTarget* NewTarget, E_ExecutionType Type)
{
	clearCurrentTarget();

	m_CurrentTargetActor = NewActor;
	m_CurrentTarget.SetObject(NewActor);
	m_CurrentTarget.SetInterface(NewTarget);
	m_CurrentExecutionType = Type;
}

void UC_PlayerExecutionComponent::clearCurrentTarget()
{
	if (m_CurrentTarget)
		m_CurrentTarget->setExecutionHintVisible(false);

	m_CurrentTargetActor = nullptr;
	m_CurrentTarget = nullptr;
	m_CurrentExecutionType = E_ExecutionType::None;
}

bool UC_PlayerExecutionComponent::selectExecution(E_ExecutionType Type, FS_ExecutionSelection& OutSelection)
{
	const FS_ExecutionSelection* Selection = nullptr;

	/*switch (Type)
	{
	case E_ExecutionType::PostureBreak:
		Selection = m_PostureMontage;
		break;
	case E_ExecutionType::Stealth:
		Selection = m_StealthMontage;
		break;

	default:
		return false;
	}*/

	if (!Selection || Selection->VariantIndex <= 0)
		return false;

	OutSelection.ExecutionID = Selection->ExecutionID;

	OutSelection.VariantIndex =
		(Selection->VariantIndex == 1)
		? 0
		: FMath::RandRange(0, Selection->VariantIndex - 1);

	return true;
}

bool UC_PlayerExecutionComponent::tryExecuteCurrentTarget()
{
	UE_LOG(LogTemp, Warning, TEXT("tryExecuteCurrentTarget"));

	if (!isValidCurrentTarget())
		return false;

	performExecution(
		m_OwnerPlayer,
		Cast<APawn>(m_CurrentTargetActor.Get()),
		m_CurrentExecutionType);

	return true;
}

void UC_PlayerExecutionComponent::performExecution(APawn* Instigator, APawn* Victim, E_ExecutionType Type)
{
	if (!Instigator || !Victim)
		return;

	AC_PlayerCharacter* Player = Cast<AC_PlayerCharacter>(Instigator);
	IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Victim);

	if (!Player || !Target)
		return;

	FS_ExecutionSelection Selection;
	if (!selectExecution(Type, Selection))
		return;

	Player->DisableInput(nullptr);
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->setCombatState(E_CombatState::Executing);

	/*Player->playPlayerExecutionMontage(Selection.ExecutionID, Selection.VariantIndex);
	Target->onExecutionStarted(Player, Selection.ExecutionID, Selection.VariantIndex);*/

	UE_LOG(LogTemp, Error, TEXT("call!!!"));
}

bool UC_PlayerExecutionComponent::canStartExecution() const
{
	const bool bValid = isValidCurrentTarget();

	return bValid;
}

E_ExecutionType UC_PlayerExecutionComponent::getCurrentExecutionType() const
{
	return m_CurrentExecutionType;
}

