// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MoveActionComponent.h"
#include "EnhancedInputComponent.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/ActorComponents/C_SprintComponent.h"
#include "ProjectARPG/ActorComponents/C_DodgeComponent.h"
#include "ProjectARPG/ActorComponents/C_WallActionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UC_MoveActionComponent::UC_MoveActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	m_SprintComp = CreateDefaultSubobject<UC_SprintComponent>(TEXT("SprintComp"));
	m_DodgeComp = CreateDefaultSubobject<UC_DodgeComponent>(TEXT("DodgeComp"));
	m_WallActionComp = CreateDefaultSubobject<UC_WallActionComponent>(TEXT("WallActionComp"));

	// ...
}

// Called when the game starts
void UC_MoveActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UC_MoveActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_WallActionComp)
		m_WallActionComp->tickWallCheck();

	if (m_Owner && m_Owner->getCombatState() == E_CombatState::Climb)
	{
		handleClimbclamp();
	}

	// ...
}

void UC_MoveActionComponent::init(AC_PlayerCharacter* InOwner)
{
	m_Owner = InOwner;

	if (!m_Owner)
		return;

	if (m_SprintComp)
		m_SprintComp->init(m_Owner);

	if (m_DodgeComp)
		m_DodgeComp->init(m_Owner);

	if (m_WallActionComp)
		m_WallActionComp->init(m_Owner);
}

void UC_MoveActionComponent::onSprintInput(const FInputActionInstance& Inst)
{
	if (!m_Owner)
		return;

	if (tryDodge(Inst))
		return;

	trySprint();

	UE_LOG(LogTemp, Warning, TEXT("[MoveAction] SprintInput Owner=%s"),
		m_Owner ? TEXT("Valid") : TEXT("Null"));
}

void UC_MoveActionComponent::onSprintReleased()
{
	if (m_SprintComp)
		m_SprintComp->stopSprint(E_SprintEndReason::InputReleased);
}

void UC_MoveActionComponent::interruptMovementAction()
{
	if (m_SprintComp && m_SprintComp->isSprinting())
		m_SprintComp->forceStop();

	if (m_WallActionComp && m_WallActionComp->isWallGrabbing())
		m_WallActionComp->releaseWall();
}

void UC_MoveActionComponent::wallMove(const FVector2D& Input)
{
	if (m_WallActionComp)
		m_WallActionComp->wallMove(Input);
}

bool UC_MoveActionComponent::tryDodge(const FInputActionInstance& Inst)
{
	if (!m_DodgeComp)
		return false;

	if (Inst.GetElapsedTime() > m_DodgeTapThreshold)
		return false;

	return m_DodgeComp->tryDodge();
}

bool UC_MoveActionComponent::trySprint()
{
	if (!m_SprintComp)
		return false;

	if (m_WallActionComp && m_WallActionComp->isWallGrabbing())
		return false;

	return m_SprintComp->startSprint();
}

bool UC_MoveActionComponent::tryWallGrab()
{
	if (!m_WallActionComp)
		return false;

	interruptMovementAction();

	return m_WallActionComp->tryWallGrab();
}

bool UC_MoveActionComponent::isWallGrabbing() const
{
	return m_WallActionComp && m_WallActionComp->isWallGrabbing();
}

bool UC_MoveActionComponent::canWallGrab() const
{
	return m_WallActionComp && m_WallActionComp->canWallGrab();
}

bool UC_MoveActionComponent::tryWallJumpOrClimb()
{
	if (!m_WallActionComp)
		return false;

	if (m_WallActionComp->isWallGrabbing())
		return m_WallActionComp->tryClimbUp();

	return false;
}

void UC_MoveActionComponent::handleClimbclamp()
{
	if (m_Owner->getCombatState() != E_CombatState::Climb)
		return;

	FVector Loc = m_Owner->GetActorLocation();
	const FVector& Target = m_Owner->getClimbTarget();

	if (Loc.Z > Target.Z)
	{
		Loc.Z = Target.Z;
		m_Owner->SetActorLocation(Loc, true);

		UCharacterMovementComponent* Move =
			m_Owner->GetCharacterMovement();

		if (Move)
		{
			Move->Velocity.Z = 0.f;
		}
	}
}

void UC_MoveActionComponent::onClimbFinished()
{
	m_Owner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UCharacterMovementComponent* Move = m_Owner->GetCharacterMovement();

	Move->SetMovementMode(MOVE_Walking);

	Move->GravityScale = m_Owner->getDefaultGravity();
	Move->AirControl = m_Owner->getDefaultAirControl();
	Move->Velocity = FVector::ZeroVector;
	m_Owner->onActionFinished();
	
}


