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

	if (!isTargetInExecutionRange())
		return false;

	return true;
}

bool UC_PlayerExecutionComponent::isTargetInExecutionRange() const
{
	if (!m_CurrentTargetActor.IsValid() || !m_OwnerPlayer)
		return false;

	USphereComponent* Sphere = m_OwnerPlayer->getExecutionSphere();
	if (!Sphere)
		return false;


	return Sphere->IsOverlappingActor(m_CurrentTargetActor.Get());
}

bool UC_PlayerExecutionComponent::isBehindTarget(AActor* Target)
{
	if (!m_OwnerPlayer || !Target)
		return false;

	const FVector ToPlayer = (m_OwnerPlayer->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();

	const float Dot = FVector::DotProduct(Target->GetActorForwardVector(), ToPlayer);


	return Dot < -0.5f;
}

void UC_PlayerExecutionComponent::findNewExecutionTarget()
{
	USphereComponent* ExecSphere = m_OwnerPlayer->getExecutionSphere();
	if (!ExecSphere)
		return;


	clearCurrentTarget();

	TArray<AActor*> Overlaps;
	ExecSphere->GetOverlappingActors(Overlaps);

	for (AActor* Act : Overlaps)
	{
		if (!Act->Implements<UC_ExecutionTarget>())
			continue;

		IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Act);
		if (!Target)
			continue;

		if (isBehindTarget(Act) && Target->canBeExecuted(E_ExecutionType::Stealth))
		{
			setCurrentExecutableTarget(Act, Target, E_ExecutionType::Stealth);

			Target->setExecutionHintVisible(true);
			return;
		}



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

bool UC_PlayerExecutionComponent::selectExecution(E_ExecutionType Type, FS_ExecutionContext& OutContext)
{
	OutContext.Type = Type;
	OutContext.Index = FMath::Rand();
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

void UC_PlayerExecutionComponent::forceExecute(APawn* ExecutionInstigator, APawn* Victim, E_ExecutionType Type)
{
	if (!ExecutionInstigator || !Victim)
		return;

	IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Victim);
	if (!Target)
		return;

	FS_ExecutionContext Context;
	if (!selectExecution(Type, Context))
		return;

	AC_PlayerCharacter* Player = Cast<AC_PlayerCharacter>(ExecutionInstigator);
	if (!Player)
		return;

	clearCurrentTarget();

	Player->DisableInput(nullptr);
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->setCombatState(E_CombatState::Executing);

	Context.Instigator = Player;
	Context.Victim = Victim;

	Player->playPlayerExecutionMontage(Context);
	Target->onExecutionStarted(Player, Context);

}

void UC_PlayerExecutionComponent::performExecution(APawn* Instigator, APawn* Victim, E_ExecutionType Type)
{
	if (!Instigator || !Victim)
		return;

	AC_PlayerCharacter* Player = Cast<AC_PlayerCharacter>(Instigator);
	IC_ExecutionTarget* Target = Cast<IC_ExecutionTarget>(Victim);

	if (!Player || !Target)
		return;

	FS_ExecutionContext Context;
	if (!selectExecution(Type, Context))
		return;

	Player->DisableInput(nullptr);
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->setCombatState(E_CombatState::Executing);

	Context.Instigator = Player;
	Context.Victim = Victim;

	Player->playPlayerExecutionMontage(Context);
	Target->onExecutionStarted(Player, Context);
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

