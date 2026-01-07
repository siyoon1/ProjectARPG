// Fill out your copyright notice in the Description page of Project Settings.


#include "C_DetectComponent.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "Engine/OverlapResult.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "ProjectARPG/AI/C_EnemyController.h"

// Sets default values for this component's properties
UC_DetectComponent::UC_DetectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UC_DetectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	m_pEnemy = Cast<AC_EnemyCharacter>(GetOwner());
}

// Called every frame
void UC_DetectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	detectTarget();


	// ...
}



void UC_DetectComponent::detectTarget()
{
	AActor* PrevTarget = m_DetectedTarget;
	m_DetectedTarget = nullptr;

	AActor* pOwner = GetOwner();
	if (!pOwner)
		return;

	UWorld* pWorld = GetWorld();
	if (!pWorld)
		return;

	FVector vCenter = pOwner->GetActorLocation();
	float fRadius = m_DetectDist;

	TArray<FOverlapResult> listOverlap{};

	FCollisionQueryParams Params(NAME_None, false, pOwner);

	bool bHit =
		pWorld->OverlapMultiByChannel
		(
			listOverlap,
			vCenter,
			FQuat::Identity,
			ECC_GameTraceChannel3,
			FCollisionShape::MakeSphere(fRadius),
			Params
		);

	//#ifdef DEBUG_DRAW
		//DrawDebugSphere(pWorld, vCenter, fRadius, 20, FColor::Red, false, 1.f);
	//#endif // DEBUG_DRAW

	if (!bHit)
	{
		if (PrevTarget)
			onTargetLost();

		return;
	}
		

	for (const FOverlapResult& Object : listOverlap)
	{
		AC_PlayerCharacter* pPlayer = Cast<AC_PlayerCharacter>(Object.GetActor());

		if (!pPlayer)
			continue;

		// 거리 체크
		if (!checkDist(pPlayer))
			continue;


		// FOV(시야각) 체크
		if (!checkFOV(pPlayer))
			continue;


		// 장애물 체크
		if (!checkLineOfSight(pPlayer))
			continue;


		m_DetectedTarget = pPlayer;

		break;

	}

	if (!PrevTarget && m_DetectedTarget)
	{
		onTargetDetected(m_DetectedTarget);
	}
	else if (PrevTarget && !m_DetectedTarget)
	{
		onTargetLost();
	}
}

bool UC_DetectComponent::checkDist(AC_PlayerCharacter* pPlayer)
{
	AActor* pOwner = GetOwner();
	float fDist = FVector::Dist(pPlayer->GetActorLocation(), pOwner->GetActorLocation());


	return fDist <= getAdjustedDetectDist(pPlayer);
}

bool UC_DetectComponent::checkFOV(AC_PlayerCharacter* pPlayer)
{
	AActor* pOwner = GetOwner();
	FVector vToTarget = (pPlayer->GetActorLocation() - pOwner->GetActorLocation()).GetSafeNormal();

	float fDot = FVector::DotProduct(pOwner->GetActorForwardVector(), vToTarget);

	float fMinDot = m_bIsDetecting ? 0.0f : 0.5f;

	if (pPlayer->isCrouch())
	{
		fMinDot = 0.7f;
	}

	float fDist = FVector::Dist(pPlayer->GetActorLocation(), pOwner->GetActorLocation());

	if (fDot >= fMinDot)
		return true;

	if (fDot < -0.2f && fDist < m_DetectDist * 0.2f)
		return true;

	if (fDot >= -0.8f)
	{
		if (fDist < m_DetectDist * 0.4f)
			return true;
	}

	

	DrawDebugLine(
		GetWorld(),
		pOwner->GetActorLocation(),
		pOwner->GetActorLocation() + pOwner->GetActorForwardVector() * 500.f,
		FColor::Green,
		false,
		0.1f
	);

	return false;
}

bool UC_DetectComponent::checkLineOfSight(AC_PlayerCharacter* pPlayer)
{
	AActor* pOwner = GetOwner();

	UWorld* pWorld = GetWorld();

	FHitResult Hit{};
	FCollisionQueryParams Params;

	Params.AddIgnoredActor(pOwner);
	Params.AddIgnoredActor(pPlayer);

	bool bBlocked =
		pWorld->LineTraceSingleByChannel
		(
			Hit,
			pOwner->GetActorLocation() + FVector(0.f, 0.f, 50.f),
			pPlayer->GetActorLocation() + FVector(0.f, 0.f, 50.f),
			ECC_Visibility,
			Params
		);

	return !bBlocked;
}

float UC_DetectComponent::getAdjustedDetectDist(AC_PlayerCharacter* pPlayer)
{
	float fDetectDist = m_DetectDist;

	if (pPlayer->isCrouch())
		fDetectDist *= 0.5f;

	if (pPlayer->getCombatState() == E_CombatState::Sprinting)
		fDetectDist *= 1.3f;

	return fDetectDist;
}

void UC_DetectComponent::onTargetDetected(AActor* NewTarget)
{
	m_bIsDetecting = true;

	if (AAIController* AICon = Cast<AAIController>(m_pEnemy->GetController()))
	{
		AICon->StopMovement();
	}

	if (m_pEnemy)
	{
		m_pEnemy->onCombatStarted();
	}
}

void UC_DetectComponent::onTargetLost()
{
	m_bIsDetecting = false;

	if (m_pEnemy)
	{
		m_pEnemy->onCombatEnded();
	}
}

void UC_DetectComponent::forceDetect(AActor* pTarget)
{
	m_DetectedTarget = pTarget;
	m_bIsDetecting = true;

	if (m_pEnemy)
	{
		m_pEnemy->onCombatStarted();
	}
}




