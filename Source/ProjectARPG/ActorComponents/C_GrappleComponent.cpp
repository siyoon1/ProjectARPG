// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrappleComponent.h"
#include "ProjectARPG/Actor/C_GrapplePoint.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

void UC_GrappleComponent::endPull()
{
	m_bIsPulling = false;
	m_pCurrentTarget = nullptr;
	m_pOwner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

bool UC_GrappleComponent::isInView(UCameraComponent* pCamera, AC_GrapplePoint* pTarget)
{
	FVector vToTarget = (pTarget->GetActorLocation() - pCamera->GetComponentLocation()).GetSafeNormal();
	float fDot = FVector::DotProduct(pCamera->GetForwardVector(), vToTarget);

	return fDot > 0.7f;
}

AC_GrapplePoint* UC_GrappleComponent::findBestGrapplePoint()
{
	TArray<AActor*> arrActors{};

	float fRadius = 2000.f;

	UKismetSystemLibrary::SphereOverlapActors
	(
		GetWorld(),
		m_pOwner->GetActorLocation(),
		fRadius,
		{UEngineTypes::ConvertToObjectType(ECC_WorldDynamic)},
		AC_GrapplePoint::StaticClass(),
		{m_pOwner},
		arrActors
	);

	UCameraComponent* Camera = m_pOwner->getFollowCamera();

	AC_GrapplePoint* pBest = nullptr;
	float fBestDist = FLT_MAX;

	for (AActor* pAct : arrActors)
	{
		AC_GrapplePoint* GP = Cast<AC_GrapplePoint>(pAct);

		if (!GP)
			continue;

		if (!isInView(Camera, GP))
			continue;

		float fDist = FVector::Dist(m_pOwner->GetActorLocation(), GP->GetActorLocation());
		if (fDist < fBestDist)
		{
			fBestDist = fDist;
			pBest = GP;
		}
	}

	return pBest;
}

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


void UC_GrappleComponent::tryStartGrapple()
{
	m_pOwner = Cast<AC_PlayerCharacter>(GetOwner());

	if (!m_pOwner)
		return;
	
	AC_GrapplePoint* pTarget = findBestGrapplePoint();

	if (pTarget)
	{
		startPull(pTarget);
		DrawDebugLine(GetWorld(), m_pOwner->GetActorLocation(), pTarget->GetActorLocation(), FColor::Green, false, 1.f);
	}
		

	

}

void UC_GrappleComponent::startPull(AC_GrapplePoint* pTarget)
{
	m_pCurrentTarget = pTarget;

	m_pOwner = Cast<AC_PlayerCharacter>(GetOwner());

	if (!m_pOwner)
		return;

	m_vOwnerPos = m_pOwner->GetActorLocation();
	m_vTargetPos = m_pCurrentTarget->GetActorLocation();

	m_bIsPulling = true;
	m_pOwner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);


	m_vPullDir = (m_vTargetPos - m_vOwnerPos).GetSafeNormal();

	m_fElapsed = 0.f;
	m_fDuration = 0.8f;
}

// Called every frame
void UC_GrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (!m_bIsPulling || !m_pOwner || !m_pCurrentTarget)
		return;

	m_fElapsed += DeltaTime;

	float Alpha = FMath::Clamp(m_fElapsed / m_fDuration, 0.f, 1.f);

	float fSpeed = 1200.f;

	FVector vCurPos = m_pOwner->GetActorLocation();
	FVector vNextPos = vCurPos + m_vPullDir * fSpeed * DeltaTime;


	FVector Pos = FMath::Lerp(vCurPos, vNextPos, Alpha);

	float Height = 300.f;
	float ZOffset = Height * FMath::Sin(Alpha * PI);
	Pos.Z += ZOffset;


	m_pOwner->SetActorLocation(vNextPos, false);
	
	float fDistToTarget = FVector::Dist(vNextPos, m_vTargetPos);

	if (fDistToTarget < 80.f && Alpha >= 1.f)
	{
		endPull();
	}

}

