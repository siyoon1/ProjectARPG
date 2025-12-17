// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrappleComponent.h"
#include "ProjectARPG/Actor/C_GrapplePoint.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CableComponent.h"
#include "Components/CapsuleComponent.h"

void UC_GrappleComponent::endPull()
{

	m_bIsPulling = false;
	m_pCable->SetVisibility(false);
	m_pCurrentTarget = nullptr;
	m_pOwner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	m_pOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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

	if (fDist < 150.f || fDist > 2000.f)
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
		ECC_Visibility,
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
	if (!m_pOwner || !m_pCable)
		return;

	m_pCurrentTarget = pTarget;

	m_bIsFiringRope = true;
	m_fRopeFireAlpha = 0.f;

	m_vRopeFireStart = m_pOwner->GetMesh()->GetSocketLocation("hand_r");
	m_vRopeFireEnd = pTarget->GetActorLocation();

	m_pCable->SetVisibility(true);
	m_pCable->bAttachEnd = false;
	m_pCable->EndLocation = FVector::ZeroVector;
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

	m_pCable = NewObject<UCableComponent>(this, UCableComponent::StaticClass());

	m_pCable->RegisterComponent();

	// 소켓 부착 금지 ? 월드에 둔다
	m_pCable->AttachToComponent(
		m_pOwner->GetRootComponent(),
		FAttachmentTransformRules::KeepWorldTransform
	);

	m_pCable->bAttachEnd = false;
	m_pCable->CableLength = 300.f;
	m_pCable->NumSegments = 4;
	m_pCable->CableWidth = 3.f;
	m_pCable->CableGravityScale = 0.f;
	m_pCable->bEnableCollision = false;
	m_pCable->bEnableStiffness = true;
	m_pCable->SolverIterations = 16;
	m_pCable->SetVisibility(false);
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

	float MinPullDistance = 150.f; // 코앞에서 안되게 하고 싶은 거리
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
	m_pCurrentTarget = pTarget;

	if (!m_pOwner)
		return;

	if (m_pCable)
	{
		m_pCable->bAttachEnd = true;
		m_pCable->SetAttachEndToComponent(
			pTarget->GetRootComponent(),
			NAME_None
		);
	}

	m_vOwnerPos = m_pOwner->GetActorLocation();
	m_vTargetPos = m_pCurrentTarget->GetActorLocation();


	m_pOwner->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	m_pOwner->GetCharacterMovement()->StopMovementImmediately();

	m_bIsPulling = true;
	m_pOwner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	m_pOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	m_vPullDir = (m_vTargetPos - m_vOwnerPos).GetSafeNormal();


	m_fElapsed = 0.f;
	m_fDuration = 0.8f;
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

		FVector HandPos = m_pOwner->GetMesh()->GetSocketLocation("hand_r");
		m_pCable->SetWorldLocation(HandPos);

		m_fRopeFireAlpha += m_fRopeFireSpeed * DeltaTime;
		float fAlpha = FMath::Clamp(m_fRopeFireAlpha, 0.f, 1.f);


		if (fAlpha >= 1.f)
		{
			m_bIsFiringRope = false;

			m_pCable->bAttachEnd = true;
			m_pCable->SetAttachEndToComponent(
				m_pCurrentTarget->GetRootComponent(),
				NAME_None
			);

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
	FRotator NewRot = FMath::RInterpTo(m_pOwner->GetActorRotation(), TargetRot, DeltaTime, 10.f);
	m_pOwner->SetActorRotation(NewRot);

	m_pOwner->SetActorLocation(Pos, false);

	if (Alpha >= 1.0f || m_pCurrentTarget->isOverlapSphere())
	{
		endPull();
	}

}

