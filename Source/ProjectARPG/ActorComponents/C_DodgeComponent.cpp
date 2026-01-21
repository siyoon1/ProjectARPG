// Fill out your copyright notice in the Description page of Project Settings.


#include "C_DodgeComponent.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Animation/C_PlayerAnim.h"

// Sets default values for this component's properties
UC_DodgeComponent::UC_DodgeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_DodgeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UC_DodgeComponent::init(AC_PlayerCharacter* InOwner)
{
	m_Owner = InOwner;
}

bool UC_DodgeComponent::tryDodge()
{
	if (!m_Owner)
		return false;	

	if (m_Owner->getActionState() != E_ActionState::Free)
		return false;

	if (m_Owner->getCombatState() != E_CombatState::Idle)
		return false;

	UC_PlayerAnim* pAnim = Cast<UC_PlayerAnim>(m_Owner->GetMesh()->GetAnimInstance());
	if (!pAnim)
		return false;

	// 회피 실행
	FVector vInputDir = m_Owner->GetLastMovementInputVector().GetSafeNormal();

	FVector vForward = m_Owner->GetActorForwardVector();
	FVector vRight = m_Owner->GetActorRightVector();

	float fForwardDot = FVector::DotProduct(vForward, vInputDir);
	float fRightDot = FVector::DotProduct(vRight, vInputDir);

	E_Direction eDir = E_Direction::Backward;

	if (!vInputDir.IsNearlyZero())
	{
		if (FMath::Abs(fForwardDot) > FMath::Abs(fRightDot))
		{
			eDir = (fForwardDot > 0) ? E_Direction::Forward : E_Direction::Backward;
		}
		else
		{
			eDir = (fRightDot > 0) ? E_Direction::Right : E_Direction::Left;
		}
	}

	m_Owner->setActionState(E_ActionState::Locked);
	m_Owner->setCombatState(E_CombatState::Dodging);


	pAnim->playDodgeMontage(eDir);

	return true;
}


// Called every frame
void UC_DodgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

