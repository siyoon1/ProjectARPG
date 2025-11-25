// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrappleComponent.h"
#include "ProjectARPG/Actor/C_GrapplePoint.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"

// Sets default values for this component's properties
UC_GrappleComponent::UC_GrappleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_GrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UC_GrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_GrappleComponent::tryStartGrapple()
{
	m_pOwner = Cast<AC_PlayerCharacter>(GetOwner());

	if (!m_pOwner)
		return;

	FVector vStart = m_pOwner->GetActorLocation();
	FVector vEnd = vStart + m_pOwner->GetActorForwardVector() * 3000.f;

	FHitResult HitResult{};
	FCollisionQueryParams Params{};

	Params.AddIgnoredActor(m_pOwner);

	bool bHit =
		GetWorld()->LineTraceSingleByChannel
		(
			HitResult,
			vStart,
			vEnd,
			ECC_Visibility,
			Params
		);

	if (bHit)
	{
		if (AC_GrapplePoint* GP = Cast<AC_GrapplePoint>(HitResult.GetActor()))
		{
			startPull(GP);
		}	

	}

}

void UC_GrappleComponent::startPull(AC_GrapplePoint* pTarget)
{
	m_pCurrentTarget = pTarget;

	m_pOwner = Cast<AC_PlayerCharacter>(GetOwner());

	if (!m_pOwner)
		return;

	FVector vOwnerPos = m_pOwner->GetActorLocation();
	FVector vTargetPos = m_pCurrentTarget->GetActorLocation();


}

