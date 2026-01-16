// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrappleComponent.h"
#include "ProjectARPG/Actor/C_GrapplePoint.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"

void UC_GrappleComponent::endPull()
{
	m_bIsPulling = false;

	if (m_pBeamComp)
	{
		m_pBeamComp->Deactivate();
		m_pBeamComp = nullptr;
	}

	m_pCurrentTarget = nullptr;

	FHitResult Hit;
	FVector Start = m_pOwner->GetActorLocation();
	FVector End = Start - FVector(0, 0, 500.f);

	if (GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility))
	{
		m_pOwner->SetActorLocation(Hit.Location);
	}

	m_pOwner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	m_pOwner->initJump();
	m_bCachedCanGrapple = false;
}

bool UC_GrappleComponent::isInView(UCameraComponent* pCamera, AC_GrapplePoint* pTarget)
{
	FVector vToTarget = (pTarget->GetActorLocation() - pCamera->GetComponentLocation()).GetSafeNormal();
	float fDot = FVector::DotProduct(pCamera->GetForwardVector(), vToTarget);

	return fDot > 0.7f;
}

bool UC_GrappleComponent::canGrapple(AC_GrapplePoint*& outTarget)
{
	outTarget = nullptr;

	if (!m_pOwner)
		return false;

	if (m_bIsPulling || m_bIsFiringRope)
		return false;

	AC_GrapplePoint* pTarget = findBestGrapplePoint();

	if (!pTarget)
		return false;

	float fDist = FVector::Dist(m_pOwner->GetActorLocation(), pTarget->GetActorLocation());

	if (fDist < 500.f || fDist > 2000.f)
		return false;

	if (!hasLineOfSight(pTarget))
		return false;

	outTarget = pTarget;
	return true;
}

bool UC_GrappleComponent::hasLineOfSight(AC_GrapplePoint* pTarget) const
{
	FHitResult HitResult{};

	FVector vStart = m_pOwner->getFollowCamera()->GetComponentLocation();
	FVector vEnd = pTarget->GetActorLocation();

	FCollisionQueryParams Params{};

	Params.AddIgnoredActor(m_pOwner);


	bool bHit = GetWorld()->LineTraceSingleByChannel
	(
		HitResult,
		vStart,
		vEnd,
		ECC_GameTraceChannel5,
		Params
	);


	if (!bHit)
		return true;


	return HitResult.GetActor() == pTarget;
}

AC_GrapplePoint* UC_GrappleComponent::findBestGrapplePoint()
{
	TArray<AActor*> arrActors{};
	float fMaxGrappleDistance = 2000.f;
	float fMinGrappleDistance = 120.f;


	UKismetSystemLibrary::SphereOverlapActors
	(
		GetWorld(),
		m_pOwner->GetActorLocation(),
		fMaxGrappleDistance,
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

		float fDist = FVector::Dist2D(m_pOwner->GetActorLocation(), GP->GetActorLocation());

		if (fDist < fMinGrappleDistance)
			continue;

		if (fDist < fBestDist)
		{
			fBestDist = fDist;
			pBest = GP;
		}
	}

	return pBest;
}

void UC_GrappleComponent::startFireRope(AC_GrapplePoint* pTarget)
{
	if (!m_pOwner || !pTarget || !m_pGrappleBeamSystem)
		return;

	m_pCurrentTarget = pTarget;

	m_bIsFiringRope = true;
	m_fRopeFireAlpha = 0.f;

	m_vRopeFireStart = m_pOwner->GetMesh()->GetSocketLocation("hand_r");
	m_vRopeFireEnd = pTarget->GetActorLocation();

	m_pBeamComp = UNiagaraFunctionLibrary::SpawnSystemAttached
	(
		m_pGrappleBeamSystem,
		m_pOwner->GetMesh(),
		"hand_r",
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);

	m_pBeamComp->SetVectorParameter
	(
		"TargetPos",
		m_pOwner->GetMesh()->GetSocketLocation("hand_r")
	);

	m_pBeamComp->SetFloatParameter("BeamAlpha", 0.f);

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
	m_bCachedCanGrapple = false;

	m_pOwner = Cast<AC_PlayerCharacter>(GetOwner());
	if (!m_pOwner)
		return;
}


void UC_GrappleComponent::tryStartGrapple()
{
	if (isPulling())
		return;

	m_pOwner = Cast<AC_PlayerCharacter>(GetOwner());

	if (!m_pOwner)
		return;
	
	AC_GrapplePoint* pTarget = findBestGrapplePoint();

	if (!pTarget)
		return;


	float dist = FVector::Dist2D(m_pOwner->GetActorLocation(), pTarget->GetActorLocation());

	float MinPullDistance = 500.f; // 코앞에서 안되게 하고 싶은 거리
	if (dist < MinPullDistance)
		return; // 너무 가까우면 Pulling 금지

	
	startFireRope(pTarget);



	if (UAnimInstance* pAnim = Cast<UAnimInstance>(m_pOwner->GetMesh()->GetAnimInstance()))
	{
		pAnim->Montage_Play(m_pGrappleStartMontage);
	}


}

void UC_GrappleComponent::startPull(AC_GrapplePoint* pTarget)
{
	if (!m_pOwner || !pTarget)
		return;

	if (m_pOwner->getCombatState() == E_CombatState::WallGrabbing)
		m_pOwner->initWallgrab();

	m_bIsPulling = true;
	m_pCurrentTarget = pTarget;

	m_vOwnerPos = m_pOwner->GetActorLocation();
	m_vTargetPos = m_pCurrentTarget->GetActorLocation();

	m_fElapsed = 0.f;
	m_fDuration = 0.8f;


	m_pOwner->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	m_pOwner->GetCharacterMovement()->StopMovementImmediately();
	m_pOwner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	m_pBeamComp->SetVectorParameter("TargetPos", m_vTargetPos);
	m_pBeamComp->SetFloatParameter("BeamAlpha", 1.f);
}

bool UC_GrappleComponent::isPulling() const
{
	return m_bIsPulling;
}

// Called every frame
void UC_GrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AC_GrapplePoint* pTarget = nullptr;
	bool bCanGrapple = canGrapple(pTarget);

	if (bCanGrapple != m_bCachedCanGrapple)
	{
		m_bCachedCanGrapple = bCanGrapple;
		m_onGrappleStateChanged.Broadcast(bCanGrapple, pTarget);
	}

	// 로프 발사


	if (m_bIsFiringRope)
	{
		FRotator TargetRot = (m_vTargetPos - m_pOwner->GetActorLocation()).Rotation();
		FRotator rNewRot = FMath::RInterpTo(m_pOwner->GetActorRotation(), TargetRot, GetWorld()->GetDeltaSeconds(), 10.f);
		m_pOwner->SetActorRotation(rNewRot);

		m_fRopeFireAlpha += m_fRopeFireSpeed * DeltaTime;
		float fAlpha = FMath::Clamp(m_fRopeFireAlpha, 0.f, 1.f);

		FVector vStart = m_pOwner->GetMesh()->GetSocketLocation("hand_r");
		FVector vEnd = m_pCurrentTarget->GetActorLocation();
		FVector vCur = FMath::Lerp(vStart, vEnd, fAlpha);

		m_pBeamComp->SetVectorParameter("TargetPos", vCur);
		m_pBeamComp->SetFloatParameter("BeamAlpha", fAlpha);


		if (fAlpha >= 1.f)
		{
			m_bIsFiringRope = false;

			startPull(m_pCurrentTarget);
		}

		return;
	}

	if (!m_bIsPulling || !m_pOwner || !m_pCurrentTarget)
		return;

	m_fElapsed += DeltaTime;
	float Alpha = FMath::Clamp(m_fElapsed / m_fDuration, 0.f, 1.f);

	// 기본 Lerp 이동 (시작→목적지 까지 정확히 도달)
	FVector Pos = FMath::Lerp(m_vOwnerPos, m_vTargetPos, Alpha);

	// 곡선 이동
	float Height = 300.f;
	Pos.Z += Height * FMath::Sin(Alpha * PI);


	FRotator TargetRot = (m_vTargetPos - m_pOwner->GetActorLocation()).Rotation();
	m_pOwner->SetActorLocation(Pos, false);
	m_pOwner->SetActorRotation(TargetRot);

	m_pBeamComp->SetFloatParameter(
		"NoiseStrength",
		(1.f - Alpha)
	);

	if (Alpha >= 1.0f || m_pCurrentTarget->isOverlapSphere())
	{
		endPull();
	}

}

