// Fill out your copyright notice in the Description page of Project Settings.


#include "C_LockOnComponent.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UC_LockOnComponent::UC_LockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_LockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	m_OwnerPawn = Cast<APawn>(GetOwner());
}


// Called every frame
void UC_LockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_LockOnComponent::toggleLockOn()
{
	if (m_CurrentTarget.IsValid())
	{
		clearLockOn();
		return;
	}

	AActor* Target = findTarget();
	if (!Target)
		return;

	m_CurrentTarget = Target;


}

void UC_LockOnComponent::clearLockOn()
{

}

bool UC_LockOnComponent::isLockOn() const
{
	return false;
}

AActor* UC_LockOnComponent::getCurrentTarget() const
{
	return nullptr;
}

bool UC_LockOnComponent::getLockOnRotation(FRotator& OutRot) const
{
	if (!m_CurrentTarget.IsValid())
		return false;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

	if (!PC)
		return false;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector Dir = (m_CurrentTarget->GetActorLocation() - CamLoc).GetSafeNormal();
	OutRot = Dir.Rotation();
	OutRot.Pitch -= 15.f;

	return true;
}

AActor* UC_LockOnComponent::findTarget()
{
	if (!m_OwnerPawn)
		return nullptr;

	APlayerController* PC = Cast<APlayerController>(m_OwnerPawn->GetController());
	if (!PC)
		return nullptr;

	FVector CamLoc;
	FRotator CamRot;

	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector CamForward = CamRot.Vector();

	TArray<FOverlapResult> Results; {};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(m_OwnerPawn);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		Results,
		m_OwnerPawn->GetActorLocation(),
		FQuat::Identity,
		ECC_GameTraceChannel3,
		FCollisionShape::MakeSphere(600.f),
		Params
	);

	if (!bHit)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	for (auto& Hit : Results)
	{
		AActor* Target = Hit.GetActor();
		if (!Target)
			continue;

		FVector Dir = (Target->GetActorLocation() - CamLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(CamForward, Dir);

		if (Dot > BestScore)
		{
			BestScore = Dot;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

